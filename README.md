# MangeMeeya_Unicode

MangeMeeya_Unicode is a Susie Plugin for MangaMeeya (Not MangaMeeyaCE) which allows it to open files with Unicode only characters.

This plugin works by modifying the IAT (Import Address Table) of the host process and redirect to our own wrappers when accessing CreateFileA and GetFileAttributesA WINAPI.

When our wrappers find that the input filename has ? (Question Mark) characters, it means the file name is invalid with current codepage.

If that happens, the plugin will use FindFirstFileW API to get the first matching file and run the Wide Char variant of the original functions and return the result to the caller.

### Basic Information

**Project Language**: Visual C

**Project Version**: Visual Studio 2010

### Usage

To use MangeMeeya_Unicode, first copy Unicode.spi to SusiePlugin folder in MangeMeeya, then run MangeMeeya, enter Environment Settings -> Loader Settings and set Susie Plugin Settings to
"Use Susie Plugin (Susie Plugin Prioritized)".

Then Unicode only files can be opened directly in MangeMeeya.

### Notes / Limitations

* Due to the wrapper functions use the first result from FindFirstFileW, that means if there are 2 files differ with the only single Unicode-only character, then the first file will always be used
* I don't know whether it is by design or a bug of Susie plugin handling in MangeMeeya, opening a file with Unicode only filename from MRU list doesn't work at the first time, because the Susie plugins will be opened AFTER it opened the archive file. However it works for the open dialog or drag and drop a file to MangaMeeya.exe or an opened MangaMeeya. To workaround the issue just open the file again from MRU list.

### Related Article
[Article on Studio KUMA](https://www.studiokuma.com/s9y/index.php?/archives/317-MangeMeeya-Unicode.html)
