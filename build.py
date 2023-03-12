#!/bin/python
import json
import sys
import os
import platform

DEF_CONF = "build_conf.json"
CMP_CMDS = "compile_commands.json"

class Builder:
	def __init__(self, config):
		if platform.system().lower() == "windows":
			self.slash = "\\"
		elif platform.system().lower() == "linux":
			self.slash = "/"

		self.isolate_lib = {
			"windows": ["mingw32", "SDL2main", "SDL2", "SDL2_image", "m",
						"glu32", "opengl32", "User32", "Gdi32", "Shell32", "glew32"],
			"linux"  : ["SDL2main", "SDL2", "SDL2_image", 
					    "m", "GL", "GLU", "GLEW"]
		}

		self.isolate_includes = {
			"windows": [f"includes{self.slash}", f"vendor{self.slash}GLEW{self.slash}include{self.slash}", f"vendor{self.slash}SDL2_64bit{self.slash}include{self.slash}", f"isolate{self.slash}includes"],
			"linux"  : [f"includes{self.slash}", f"{self.slash}usr{self.slash}include{self.slash}", f"isolate{self.slash}includes"]
		}

		self.isolate_lib_path = {
			"windows": [f"vendor{self.slash}GLEW{self.slash}lib{self.slash}", f"vendor{self.slash}SDL2_64bit{self.slash}lib{self.slash}"],
			"linux"  : [f"{self.slash}usr{self.slash}lib{self.slash}"]
		}

		self.vendor_dlls = [f"vendor{self.slash}GLEW{self.slash}bin{self.slash}", f"vendor{self.slash}SDL2_64bit{self.slash}bin{self.slash}"]

		self.load_config(config)

	def load_config(self, config):
		self.exec_dir    = self.slash.join(os.path.abspath(__name__).split(self.slash)[:-1]) + self.slash
		self.project_dir = self.slash.join(os.path.abspath(config).split(self.slash)[:-1]) + self.slash
		self.config = json.load(open(config, "r"));

		# Loading the data
		self.platform = self.config["platform"]

		if self.platform == "auto":
			self.platform = platform.system().lower()

		self.isolate_path = self.config["isolate_path"]
		self.out_dir = self.project_dir + "bin"
		self.out = self.out_dir + self.slash + self.config["out"]
		self.cc  = self.config["cc"]

		# Compiler flags
		self.c_flags = " ".join(self.config["c_flags"])
		# self.c_files = " ".join(map((self.project_dir + "{0}").format, self.config["c_files"]))
		self.c_files = list(map((self.project_dir + "{0}").format, self.config["c_files"]))
		self.o_files = list(map((self.project_dir + "{0}.o").format, self.config["c_files"]))

		# Includes and libraries
		self.includes = (
			" ".join(map(("-I" + self.isolate_path + "{0}").format, self.isolate_includes[self.platform]))
		  + " ".join(map("-I{0}".format, self.config["includes"]))
		)

		self.lib_path = (
			" ".join(map(("-L" + self.isolate_path + "{0}").format, self.isolate_lib_path[self.platform]))
		  + " ".join(map("-L{0}".format, self.config["lib_path"]))
		)

		self.lib = (
			" ".join(map("-l{0}".format, self.isolate_lib[self.platform]))
		  + " ".join(map("-l{0}".format, self.config["lib"]))
		)

	def exec(self, cmd):
		print("[CMD]:", cmd, end="\n\n")
		os.system(cmd)

	def __generate_cmp_cmds(self):
		cmp_cmds = []
		if os.path.isfile(CMP_CMDS):
			cmp_cmds = json.load(open(CMP_CMDS, "r"))

		for c_file, o_file in zip(self.c_files, self.o_files):
			cmd = f"{self.cc} {self.c_flags} {self.includes} -o {o_file} -c {c_file}"
			section = {
				"directory": self.exec_dir,
				"command": cmd, 
				"file": c_file
			}
			if section not in cmp_cmds:
				cmp_cmds.append(section)

		json.dump(cmp_cmds, open(CMP_CMDS, "w"), indent=4)

	def __compile(self):
		# Compiling
		for c_file, o_file in zip(self.c_files, self.o_files):
			cmp_cmd = f"{self.cc} {self.c_flags} {self.includes} -o {o_file} -c {c_file}"
			self.exec(cmp_cmd)

		# Making build directory
		if not os.path.exists(self.out_dir):
			os.mkdir(self.out_dir)

		# Building
		files = " ".join(self.o_files)
		build_cmd = f"{self.cc} -o {self.out} {files} {self.lib_path} {self.lib}"
		self.exec(build_cmd)

	def __compile_windows(self):
		self.__compile()

		# Copying dlls
		for dll in self.vendor_dlls:
			self.exec(f"copy {self.isolate_path}{dll}*.dll {self.out_dir}")

		# Cleaning
		files = " ".join(self.o_files)
		self.exec(f"erase {files}")

	def __compile_linux(self):
		self.__compile()
		files = " ".join(self.o_files)
		self.exec(f"rm {files}")

	def run(self):
		if self.platform == "windows":
			self.__compile_windows()
		elif self.platform == "linux":
			self.__compile_linux()

		self.__generate_cmp_cmds()


if __name__ == "__main__":
	if len(sys.argv) == 1:
		config_file = DEF_CONF
	else:
		config_file = sys.argv[1]

	builder = Builder(config_file)
	builder.run()
