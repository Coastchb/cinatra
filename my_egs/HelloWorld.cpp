#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <sndfile.hh>

#include "../include/cinatra.hpp"
#include "cinatra/metric_conf.hpp"

using namespace cinatra;
using namespace ylt::metric;
using namespace std::chrono_literals;

void create_file(std::string filename, size_t file_size = 64) {
  std::ofstream file(filename, std::ios::binary);
  if (file) {
    std::string str(file_size, 'C');
    file.write(str.data(), str.size());
  }
}

async_simple::coro::Lazy<void> byte_ranges_download() {
  create_file("my_test_file.txt", 64);
  create_file("a.txt", 64);
  create_file("b.txt", 64);
  create_file("c.txt", 64);
  coro_http_server server(1, 8090);
  server.set_static_res_dir("", "");
  server.async_start();
  std::this_thread::sleep_for(200000s);
}


async_simple::coro::Lazy<void> download_txt_file() {
		int max_thread_num = std::thread::hardware_concurrency();
		coro_http_server server(max_thread_num, 8080);
		server.set_http_handler<GET, POST>("/TTS", [](coro_http_request& req, coro_http_response& res) {
      /*
      for (auto &[k, v] : req.get_headers()) {
        std::cout << "k:" << k << ";v:" << v << std::endl;
      }
      std::cout << "get_body:" << req.get_body() << std::endl;
      std::cout << "req.get_method():" << req.get_method() << std::endl;
      for (auto &[q, v] : req.get_queries()) {
        std::cout << "q:" << q << ";v:" << v << std::endl;
      }*/

      std::string file_name = std::string(req.get_query_value("file"));
      std::cout << "download file:" << file_name << std::endl;
      std::ifstream file(file_name); // 打开文件
      std::string content = "";
      if(file.is_open()) { // 检查文件是否成功打开
          std::string line;
          while(getline(file, line)) { // 逐行读取文件内容
              std::cout << line << std::endl; // 输出每一行内容
              content += line;
          }
          file.close(); // 关闭文件
      }
      else {
          std::cout << "无法打开文件" << std::endl;
          content = "无法打开文件";
      }
      
			res.set_status_and_content(status_type::ok, content);
		});

		server.sync_start();
    std::this_thread::sleep_for(200000s);
}


async_simple::coro::Lazy<void> download_wav_file_v1() {
		int max_thread_num = std::thread::hardware_concurrency();
		coro_http_server server(max_thread_num, 8080);
		server.set_http_handler<GET, POST>("/TTS", [](coro_http_request& req, coro_http_response& res) {
      SF_INFO info;
      info.format = 0;
      
      SNDFILE* file = sf_open("../output.wav", SFM_READ, &info);
      if (file == NULL) {
          std::cerr << "Error opening wav file." << std::endl;
      }
      
      // 确保我们正在处理一个二进制格式的WAV文件
      std::cout << "info.format:" << info.format << std::endl;
      std::cout << "SF_FORMAT_WAV:" << SF_FORMAT_WAV << std::endl;
      std::cout << "SF_FORMAT_PCM_16:" << SF_FORMAT_PCM_16 << std::endl;
      std::cout << "SF_FORMAT_WAV | SF_FORMAT_PCM_16:" << (SF_FORMAT_WAV | SF_FORMAT_PCM_16) << std::endl;
      if (info.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
          std::cerr << "File is not a binary WAV with 16-bit PCM." << std::endl;
          sf_close(file);
      }
      
      // 读取文件内容
      int frames = info.frames;
      std::vector<short> buffer(frames);
      int read = sf_read_short(file, &buffer[0], frames);
      
      if (read < frames) {
          std::cerr << "Error reading file." << std::endl;
          sf_close(file);
      }

      std::string content = "";
      for (auto x : buffer) {
        content += std::to_string(x);
        std::cout << x << "\t";
      }
      //std::cout << content << std::endl;
      res.set_status_and_content(status_type::ok, content);
      
      // 关闭文件
      sf_close(file);
			
		});

		server.sync_start();
    std::this_thread::sleep_for(200000s);
}



async_simple::coro::Lazy<void> download_wav_file() {
		int max_thread_num = std::thread::hardware_concurrency();
		coro_http_server server(max_thread_num, 8080);
		server.set_http_handler<GET, POST>("/TTS", [](coro_http_request& req, coro_http_response& res) {
      std::ifstream file("../output.wav", std::ios::binary | std::ios::in);
      if (!file.is_open()) {
          std::cout << "Failed to open wav file." << std::endl;
      }
    
      // 获取文件大小
      file.seekg(0, std::ios::end);
      int fileSize = file.tellg();
      file.seekg(0, std::ios::beg);
      std::cout << "fileSize:" << fileSize << std::endl;
    
      // 创建一个缓冲区来存储文件内容
      char* buffer = new char[fileSize];
    
      // 读取文件内容
      file.read(buffer, fileSize);
      file.close();

      std::string content = buffer;
      std::cout << "content:" << content << std::endl;
      std::cout << "content.size:" << content.size() << std::endl;

      // 使用读取到的数据进行操作
      res.set_status_and_content(status_type::ok, content);
      
      // 释放缓冲区内存
      delete[] buffer;
			
		});

		server.sync_start();
    std::this_thread::sleep_for(200000s);
}

async_simple::coro::Lazy<void> download_wav_file_v2(const std::string& str) {
    std::cout << "str:" << str << std::endl;
		int max_thread_num = std::thread::hardware_concurrency();
		coro_http_server server(max_thread_num, 8080);
		server.set_http_handler<GET, POST>("/TTS", [](coro_http_request& req, coro_http_response& res) {
      std::ifstream file("../output.wav", std::ios::binary);

      if (!file) {
          std::cerr << "无法打开文件" << std::endl;
      }
  
      // 读取文件全部内容到字符串流
      std::stringstream buffer;
      buffer << file.rdbuf();
      file.close();

      std::string content = buffer.str();

      // 使用读取到的数据进行操作
      res.set_status_and_content(status_type::ok, content);
			
		});

		server.sync_start();
    //std::this_thread::sleep_for(200000s);
}

int main() {
    std::string a = "Haha";
    async_simple::coro::syncAwait(download_wav_file_v2(a));
	return 0;
}