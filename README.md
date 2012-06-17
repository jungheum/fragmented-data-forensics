# frag_insight
*by Jungheum Park* 

frag_insight is a tool for fragmented data forensics.

**Supported platforms**

 * Windows (VS 2010 project, written in C++ and MFC)

## Quick start

Clone the git repo `https://github.com/jungheum/fragmented-data-forensics.git` or [download it](https://github.com/jungheum/fragmented-data-forensics/zipball/master)

Execute `frag_insight` to analyze an image file
<pre>
frag_insight  'target image'  'pagesize'  'output path'
</pre>

Examples of usage
<pre>
frag_insight  c:\\image1.dd   2048   c:\\output1
frag_insight  c:\\image2.dd   4096   c:\\output2
</pre>

## Features

#### Target
* Flash memory image (fragmented pages)
* Unallocated area of filesystem


#### Features
* Page classification
	* hash-based classification (deduplication)
	* meta page classification
		* it supports the YAFFS and EXT4 file system
	* statistical classification 
	* file format classification
		* it supports file formats such as SQLite, XML, HTML, TEXT, etc.

* Page analysis
	* format-based data analysis
		* this tool analyzes SQLite header / record

## License

[DFRC@KU](https://github.com/jungheum/fragmented-data-forensics/blob/master/COPYING)

## Feedback

Please submit feedback via the frag_insight [tracker](http://github.com/jungheum/fragmented-data-forensics/issues)

Author: Jungheum Park (junghmi@gmail.com)