//Make sure to define header once
#pragma once

#include <iostream>
#include <future>
#include <functional>
#include <chrono>
#include <unistd.h>

using namespace std;

//Create main stream macros
#define MS *common::Program_stream::main_stream
#define MS_OUT common::Program_stream::main_stream->out_obj()
#define MS_ERR common::Program_stream::main_stream->err_obj()
#define MS_IN common::Program_stream::main_stream->in_obj()

namespace common{
	//Program exit codes
	enum EXIT_CODE {SUCCESS, FAILURE};
	
	//Exit status
	struct EXIT_STATUS{
		EXIT_CODE code;
		string message;
		
		EXIT_STATUS(EXIT_CODE _code, string _message): code(_code), message(_message) {}
	};
	
	//Program in and out stream
	class Program_stream{
	private:
		//In and out declaration
		ostream &out;
		ostream &err;
		istream &in;
	public:
		//Pointer to operative stream
		static Program_stream* main_stream;
		
		//Bind streams
		Program_stream(ostream& output_s, ostream& error_s, istream& input_s): out(output_s), err(error_s), in(input_s) {
			main_stream = this;
		}
		
		void change_operative_stream(Program_stream *new_stream) {
			main_stream = new_stream;
		}
		
		//Return reference to each stream
		ostream& out_obj() { return out; }
		ostream& err_obj() { return err; }
		istream& in_obj() { return in; }
		
		//Overload out operator for values
		template<class T>
		Program_stream& operator<<(T const& value){
			out << value;
			return *this;
		}
		
		//Overload out operator for stream modifiers
		Program_stream& operator<<(ostream&(func)(ostream&)) {
			out << func;
			return *this;
		}
		
		//Overload in operator
		template<class T>
		Program_stream& operator>>(T& data){
			in >> data;
			return *this;
		}
	};
	
	//Store main stream
	Program_stream* Program_stream::main_stream = nullptr;
	
	namespace functionality{
		//Loading class
		template<class R>
		class Loading{
		private:
			//Store task
			future<R> task;
		
			//Writ loading screen until task done
			void Load_screen(future<R>& thread, string message) {
				//Throw exception if main_stream not created
				if(Program_stream::main_stream == nullptr) throw FAILURE;
				std::future_status thread_status;
				int count = 0;
				MS << message;
				do{
					count++;
					thread_status = thread.wait_for(0ms);
					MS << "." << std::flush;
					sleep(1);
					if(count-3==0) {
						count = 0;
						MS << "\b\b\b   \b\b\b" << std::flush;
					}
				} while(thread_status != std::future_status::ready);
				MS << std::endl;
			}
		public:
			//Run loading
			Loading(function<R()> lambda, string message = "Loading"): task(async(launch::async, lambda)) {
				Load_screen(task, message);
			}
			
			//Get task result
			R get_result() { return task.get(); }
			
			//Destroy loader if needed
			void destroy() { delete this; }
		};
		
		template<class R>
		R fast_load(function<R()> lambda, string message = "Loading") {
			R result;
			try{
				Loading<R>* new_loader = new Loading<R>(lambda, message);
				result = new_loader->get_result();
				delete new_loader;
			}catch(EXIT_STATUS& EX) { throw EX; };
			return result;
		}
		
		//Ask yes or no question
		bool bool_ask(string message) {
			//Throw exception if main_stream not created
			if(Program_stream::main_stream == nullptr) throw FAILURE;
			char choice;
			MS << message << "[Y/N]: ";
			MS >> choice;
			return choice == 'y' || choice == 'Y';
		}
	}
}