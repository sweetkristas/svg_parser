/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>

#if defined(__linux__)
//Avoid link error on Linux when compiling with -std=c++0x and linking with
//a Boost lib not compiled with these flags.
#define BOOST_NO_SCOPED_ENUMS
#define BOOST_NO_CXX11_SCOPED_ENUMS
#endif

#include <boost/filesystem.hpp>

#include "asserts.hpp"
#include "filesystem.hpp"

#ifdef __linux__
#include <sys/inotify.h>
#include <sys/select.h>
#endif

namespace sys
{
	using namespace boost::filesystem;

	namespace 
	{
#ifdef HAVE_CONFIG_H
		const std::string data_dir = DATADIR ;
		const bool have_datadir = true;
#else
		const std::string data_dir = "";
		const bool have_datadir = false;
#endif
	}

	bool is_directory(const std::string& dname)
	{
		return is_directory(path(dname));
	}


	void get_files_in_dir(const std::string& dir,
		std::vector<std::string>* files,
		std::vector<std::string>* dirs)
	{
		path p(dir);
		if(!is_directory(p) && !is_other(p)) {
			return;
		}
		for(directory_iterator it = directory_iterator(p); it != directory_iterator(); ++it) {
			if(is_directory(it->path()) || is_other(it->path())) {
				if(dirs != NULL) {
					dirs->push_back(it->path().filename().generic_string());
				}
			} else {
				if(files != NULL) {
					files->push_back(it->path().filename().generic_string());
				}
			}
		}
		if(files != NULL)
			std::sort(files->begin(), files->end());

		if (dirs != NULL)
			std::sort(dirs->begin(), dirs->end());
	}


	void get_unique_filenames_under_dir(const std::string& dir,
                                    std::map<std::string, std::string>* file_map,
									const std::string& prefix)
	{
		ASSERT_LOG(file_map != NULL, "get_unique_filenames_under_dir() passed a NULL file_map");
		path p(dir);
		if(!is_directory(p)) {
			return;
		}
		for(recursive_directory_iterator it = recursive_directory_iterator(p); it != recursive_directory_iterator(); ++it) {
			if(!is_directory(it->path())) {
				(*file_map)[prefix + it->path().filename().generic_string()] = it->path().generic_string();
			}
		}
	}

	std::string get_dir(const std::string& dir)
	{
		try {
			create_directory(path(dir));
		} catch(filesystem_error&) {
			return "";
		}
		return dir;
	}

	std::string read_file(const std::string& fname)
	{
		std::ifstream file(fname.c_str(), std::ios_base::binary);
		std::stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	void write_file(const std::string& fname, const std::string& data)
	{
		path p(fname);
		ASSERT_LOG(p.has_filename(), "No filename found in write_file path: " << fname);

		// Create any needed directories
		boost::system::error_code ec;
		create_directories(p.parent_path(), ec);

		// Write the file.
		std::ofstream file(fname.c_str(), std::ios_base::binary);
		file << data;
	}

	bool dir_exists(const std::string& fname)
	{
		path p(fname);
		return exists(p) && is_directory(p);
	}

	bool file_exists(const std::string& fname)
	{
		path p(fname);
		return exists(p) && is_regular_file(p);
	}

	std::string find_file(const std::string& fname)
	{
		if(file_exists(fname)) {
			return fname;
		}
		if(have_datadir) {
			std::string data_fname = data_dir + "/" + fname;
			if(file_exists(data_fname)) {
				return data_fname;
			}
		}
		return fname;
	}

	int64_t file_mod_time(const std::string& fname)
	{
		path p(fname);
		if(is_regular_file(p)) {
			return static_cast<int64_t>(last_write_time(p));
		} else {
			return 0;
		}
	}

	void move_file(const std::string& from, const std::string& to)
	{
		rename(path(from), path(to));
	}

	void remove_file(const std::string& fname)
	{
		remove(path(fname));
	}

	void copy_file(const std::string& from, const std::string& to)
	{
		copy_file(path(from), path(to), copy_option::fail_if_exists);
	}

	void rmdir_recursive(const std::string& fpath)
	{
		remove_all(path(fpath));
	}

	bool is_path_absolute(const std::string& fpath)
	{
		return path(fpath).is_absolute();
	}

	std::string make_conformal_path(const std::string& fpath)
	{
		return path(fpath).generic_string();
	}

	namespace
	{
		std::string del_substring_front(const std::string& target, const std::string& common)
		{
			if(boost::iequals(target.substr(0, common.length()), common)) {
			//if(target.find(common) == 0) {
				return target.substr(common.length());
			}
			return target;
		}

		std::string normalise_path(const std::string& path)
		{
			if(is_path_absolute(path)) { 
				return path;
			}
			std::vector<std::string> cur_path;
			std::string norm_path;
			boost::split(cur_path, path, std::bind2nd(std::equal_to<char>(), '/'));
			for(const std::string& s : cur_path) {
				if(s != ".") {
					norm_path += s + "/";
				}
			}
			return norm_path;
		}
	}

	// Calculates the path of target relative to source.
	std::string compute_relative_path(const std::string& source, const std::string& target)
	{
		//std::cerr << "compute_relative_path(a): " << source << " : " << target << std::endl;
		std::string common_part = normalise_path(source);
		std::string back;
		if(common_part.length() > 1 && common_part[common_part.length()-1] == '/') {
			common_part.erase(common_part.length()-1);
		}
		while(boost::iequals(del_substring_front(target, common_part), target)) {
			size_t offs = common_part.rfind('/');
			//std::cerr << "compute_relative_path(b2): " << back << " : " << common_part << std::endl;
			if(common_part.length() > 1 && offs != std::string::npos) {
				common_part.erase(offs);
				back = "../" + back;
			} else {
				break;
			}
		}
		common_part = del_substring_front(target, common_part);
		if(common_part.length() == 1) {
			common_part = common_part.substr(1);
			if(back.empty() == false) {
				back.erase(back.length()-1);
			}
		} else if(common_part.length() > 1 && common_part[0] == '/') {
			common_part = common_part.substr(1);
		} else {
			if(back.empty() == false) {
				back.erase(back.length()-1);
			}
		}
		//std::cerr << "compute_relative_path(b): " << back << " : " << common_part << std::endl;
		return back + common_part;
	}
}
