#!/usr/bin/env python3

from collections import namedtuple
from urllib.request import urlopen


def main():
	File = namedtuple("File", "path repo rpath")
	files = [
		File("src/stb_dxt.h", "nothings/stb", "stb_dxt.h"),
		File("src/stb_image.h", "nothings/stb", "stb_image.h")
	]

	for file in files:
		remote = f"https://raw.githubusercontent.com/{file.repo}/master/{file.rpath}"
		with urlopen(remote) as request, \
				open(file.path, "w", newline="\n") as out:
			out.write(request.read().decode("UTF-8").replace("\r\n", "\n"))


if __name__ == "__main__":
	main()
