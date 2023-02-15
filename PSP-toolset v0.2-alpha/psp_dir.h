#pragma once

#include <filesystem>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <mntent.h>

#include "psp_common.h"

namespace fs = std::filesystem;
namespace cm = common;
namespace func = common::functionality;

namespace psp_dir{	
	//Debian mount files location
	const fs::path loc_proc = "/proc/mounts";
	
	//Mount path
	const fs::path mnt = "/mnt";
	
	//Time variables in seconds
	const int timeout = 60;
	const int refresh = 2;
	
	//Check if path is psp dir
	bool check_path(const fs::path& p) {
		try{
			if(!fs::exists(p)) return false;
			//Check baisic psp dirs
			fs::path check1 = p / "MSTK_PRO.IND";
			fs::path check2 = p / "MEMSTICK.IND";
			if(!fs::exists(check1) || !fs::exists(check2)) return false;
			//Skip dirs you dont have permission to open
		} catch(fs::filesystem_error const& ex) { return false; } 
		return true;
	}
	
	//Check if device is already mounted
	bool is_mounted(const fs::path& device) {
		std::ifstream mounts(loc_proc);
		if(!mounts.is_open()) throw(cm::EXIT_STATUS(cm::FAILURE, "COULDNT OPEN /proc/mounts"));
		std::string line;
		while(std::getline(mounts, line)) {
			if(line.find(device) == 0) return true;
		}
		return false;
	}
	
	//Create vector of devices that meet the conditions
	std::vector<fs::path> check_device_vector(std::vector<fs::path> devices) {
		std::vector<fs::path> u_dev;
		for(fs::path p : devices) {
		//Check if driver device and if its mounted
			if(strncmp(p.c_str(), "/dev/sd", 7) == 0 && !is_mounted(p)) u_dev.push_back(p);
		}
		return u_dev;
	}
	
	//Mount a device but ONLY first partition
	void mount_device(fs::path dev) {
		std::string c = "mount " + dev.string() + "1 " + mnt.string();
		int ex = system(c.c_str());
		if(ex != 0) throw cm::EXIT_STATUS(cm::FAILURE, "COULDNT MOUNT " + dev.string() + "1");
	}
	
	//Unmount a device
	void unmount_device(fs::path mount_path) {
		std::string c = "umount " + mount_path.string();
		int ex = system(c.c_str());
		if(ex != 0) throw cm::EXIT_STATUS(cm::FAILURE, "COULDNT MOUNT " + mount_path.string());
	}
	
	//Get list of devices mounts
	std::vector<fs::path> mounted_devices() {
		std::vector<fs::path> paths;
		struct mntent *ent;
		FILE *mounts;
		
		//Read mount files
		mounts = setmntent(loc_proc.c_str(), "r");
		if(mounts == NULL) throw cm::EXIT_STATUS(cm::FAILURE, "SETMENT ERROR");
		
		//Push to mounts list
		while(NULL != (ent = getmntent(mounts))) {
			//Add only vfat types
			if(strncmp(ent->mnt_type, "vfat", 4) == 0)paths.push_back(ent->mnt_dir);
		}
		endmntent(mounts);
		return paths;
	}
	
	std::vector<fs::path> device_list() {
		std::vector<fs::path> devices;
		//Check block
		for(const fs::path &entry : fs::recursive_directory_iterator("/sys/block")) {
			//Check if its directory
			if(fs::is_directory(entry)) {
				const fs::path uevent_path = entry / "uevent";
				//Check if it is external device
				if(!fs::exists(entry / "device")) continue;
				//Check if uevent exist
				if(fs::exists(uevent_path)) {
					//Open uevent file
					std::ifstream uevent_file(uevent_path.c_str());
					if(uevent_file.is_open()) {
						std::string line;
						while(std::getline(uevent_file, line)) {
							//Check if device type is disk
							if(line.find("DEVTYPE=disk") != std::string::npos) {
								//Push device to result
								devices.push_back("/dev/" + entry.filename().string());
								break;
							}
						}
					}
				}
			}
		}
		return devices;
	}
	
	//Get psp dir by checking mounts
	std::string get_psp_dir_by_mounts() {
		fs::path psp_path;
		//Look for psp mount
		try{
			psp_path = func::fast_load<fs::path>([]{
				vector<fs::path> possible_paths = mounted_devices(); 
				for(fs::path p : possible_paths) {
					if(check_path(p)) return p;
				}
				throw cm::EXIT_STATUS(cm::FAILURE, "COULDN'T LOCATE PSP DIR MOUNT");
			}, "Checking mountpoins");
		} catch(cm::EXIT_STATUS& EX) { throw EX; };
		return psp_path.u8string();
	}
	
	//Get psp dir by checking unmounted devices and mounting them
	std::string get_psp_dir_by_devices() {
		fs::path psp_path;
		std::vector<fs::path> u_dev;
		//Search for unmounted devices
		try{
			u_dev = func::fast_load<std::vector<fs::path>>([]{
				std::vector<fs::path> u_dev = check_device_vector(device_list());
				return u_dev;
			}, "Looking for unmounted devices");
		} catch(cm::EXIT_STATUS& EX) { throw EX; }
		if(u_dev.empty()) u_dev = func::fast_load<std::vector<fs::path>>([]{
			int passed = 0;
			while(timeout>passed){
				std::vector<fs::path> get_devices = check_device_vector(device_list());
				if(!get_devices.empty()) return get_devices;
				sleep(refresh);
				passed += refresh;
			}
			throw cm::EXIT_STATUS(cm::FAILURE, "FAILURE DUE TO TIMEOUT");
		}, "Please plug the device to the computer");
		//Ask for device mount
		if(func::bool_ask("Found " + to_string(u_dev.size()) + " unmounted devices do you want to mount and check them?")) {
			//Check every unmounted device
			for(fs::path p : u_dev) {
				try{mount_device(p);} catch(cm::EXIT_STATUS* EX){ throw EX; }
				MS << "Mounted " << p << " to " << mnt << std::endl;
				if(check_path(mnt)) {
					psp_path = mnt;
					break;
				}
				try{unmount_device(p);} catch(cm::EXIT_STATUS* EX){ throw EX; }
				MS << "Unmounted " << p << std::endl;
			}
		}
		if(psp_path.empty()) throw cm::EXIT_STATUS(cm::FAILURE, "PSP NOT FOUND");
		return psp_path;
	}
}