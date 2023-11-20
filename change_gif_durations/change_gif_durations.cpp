
#include <iostream>
#include <string>
#ifndef FOR_LINUX
#include <Windows.h>
#else
#include <fstream>
#include <string.h>
#endif
#include "CrossPlatformDefs.h"
#include "GIF_parse.h"
#include <vector>

#ifndef FOR_LINUX
#define CrossPlatformMainName wmain
#else
#define CrossPlatformMainName main
#endif

bool fileExists(const CrossPlatformString& path) {
#ifndef FOR_LINUX
    DWORD fileAtrib = GetFileAttributesW(path.c_str());
    if (fileAtrib == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return true;
#else
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) return false;
    fclose(file);
    return true;
#endif
}

void trim(std::string& str) {
    if (str.empty()) return;
    auto it = str.end();
    --it;
    while (true) {
        if (*it >= 32) break;
        if (it == str.begin()) {
            str.clear();
            return;
        }
        --it;
    }
    str.resize(it - str.begin() + 1);
}

bool crossPlatformOpenFile(FILE** file, const CrossPlatformString& path) {
#ifndef FOR_LINUX
    if (_wfopen_s(file, path.c_str(), CrossPlatformText("r+b")) || !*file) {
        CrossPlatformPerror(path.c_str());
        if (*file) {
            fclose(*file);
        }
        return false;
    }
    return true;
#else
    * file = fopen(path.c_str(), "r+b");
    if (!*file) {
        CrossPlatformPerror(path.c_str());
        return false;
    }
    return true;
#endif
}

void crossPlatformCopyFile(const CrossPlatformString& pathSource, const CrossPlatformString& pathDestination) {
    #ifdef FOR_LINUX
    std::ifstream src(pathSource, std::ios::binary);
    std::ofstream dst(pathDestination, std::ios::binary);

    dst << src.rdbuf();
    #else
    if (!CopyFileW(pathSource.c_str(), pathDestination.c_str(), true)) {
        std::wcout << "Failed to copy from " << pathSource.c_str() << " to " << pathDestination.c_str() << std::endl;
    }
    #endif
}

std::vector<CrossPlatformString> split(const CrossPlatformString& str, CrossPlatformChar c) {
    std::vector<CrossPlatformString> result;
    const CrossPlatformChar* strStart = &str.front();
    const CrossPlatformChar* strEnd = strStart + str.size();
    const CrossPlatformChar* prevPtr = strStart;
    const CrossPlatformChar* ptr = strStart;
    while (*ptr != '\0') {
        if (*ptr == c) {
            if (ptr > prevPtr) {
                result.emplace_back(prevPtr, ptr - prevPtr);
            }
            else if (ptr == prevPtr) {
                result.emplace_back();
            }
            prevPtr = ptr + 1;
        }
        ++ptr;
    }
    if (prevPtr < strEnd) {
        result.emplace_back(prevPtr, strEnd - prevPtr);
    }
    else {
        result.emplace_back();
    }
    return result;
}

std::string wideStringToString(const std::wstring& str) {
    std::string result;
    result.reserve(str.size());
    for (auto it = str.cbegin(); it != str.cend(); ++it) {
        result.push_back((char)*it);
    }
    return result;
}

bool parseInteger(const CrossPlatformString& value, int& integer) {
    int result;
    for (auto it = value.begin(); it != value.end(); ++it) {
        if (!(*it >= CrossPlatformText('0') && *it <= CrossPlatformText('9'))) return false;  // apparently atoi doesn't do this check
    }
    #ifndef FOR_LINUX
    result = std::atoi(wideStringToString(value).c_str());
    #else
    result = std::atoi(value.c_str());
    #endif
    if (result == 0 && value != CrossPlatformText("0")) return false;
    integer = result;
    return true;
}

int findChar(const CrossPlatformString& buf, CrossPlatformChar c) {
    for (auto it = buf.cbegin(); it != buf.cend(); ++it) {
        if (*it == c) {
            return it - buf.cbegin();
        }
    }
    return -1;
}

#define PARAMETERS_FORMAT_HELP CrossPlatformText("1 - input/output file name (file will be read and modified);\n")\
	CrossPlatformText("2 - frame range in format 0-20, frame numbers starting from 0. This parameter must not be present when using -durations.\n")\
	CrossPlatformText("3 - -duration ## or -fps ##. -duration specifies time in ms between frames. -fps specifies frames per second.\n")\
	CrossPlatformText("3 - Alternatively you can specify -durations \"path\" pointing to a file")\
	CrossPlatformText(" which contains durations in ms for each frame on each new line.")\
	CrossPlatformText(" File must contain only numbers and newlines in ASCII and you can't specify a frame range.\n")\
	CrossPlatformText("\n")\
	CrossPlatformText("\nAlternative mode: shows framerate. Expects 2 arguments:\n")\
	CrossPlatformText("1 - filename\n")\
	CrossPlatformText("2 - -f. A flag (which means \"show framerate\") (don't type \"show framerate\", type the -f flag)\n")\
    CrossPlatformText("3 - -u. A flag (which means \"user-friendly\") which changes the format of the output")\
    CrossPlatformText(" because without it the default format is the same format that program expects in a file in a -durations option.\n")

int CrossPlatformMainName(int argc, CrossPlatformChar* argv[], CrossPlatformChar* envp[])
{
    if (argc == 2 && (
            CrossPlatformCaseInsensitiveTextCompare(argv[1], CrossPlatformText("-help")) == 0
            || CrossPlatformCaseInsensitiveTextCompare(argv[1], CrossPlatformText("--help")) == 0
            || CrossPlatformCaseInsensitiveTextCompare(argv[1], CrossPlatformText("/?")) == 0
            )) {
        CrossPlatformCout << CrossPlatformText("The program modifies durations of a given GIF file's frames. Expects arguments:\n") PARAMETERS_FORMAT_HELP;
        exit(0);
    }

    bool metUFlag = false;
    bool metFFlag = false;
    bool metDurationFlag = false;
    bool metFPSFlag = false;
    bool metDurationsFlag = false;
    CrossPlatformString argumentWhichIsAfterDurations;
    bool needToCaptureArgumentWhichIsAfterDurations = false;
    std::vector<CrossPlatformString> unparsedArgs;
    CrossPlatformString filename;
    for (int i = 1; i < argc; ++i) {
        if (CrossPlatformCaseInsensitiveTextCompare(argv[i], CrossPlatformText("-f")) == 0) {
            metFFlag = true;
        } else if (CrossPlatformCaseInsensitiveTextCompare(argv[i], CrossPlatformText("-u")) == 0) {
            metUFlag = true;
        }
        else if (CrossPlatformCaseInsensitiveTextCompare(argv[i], CrossPlatformText("-duration")) == 0) {
            metDurationFlag = true;
        }
        else if (CrossPlatformCaseInsensitiveTextCompare(argv[i], CrossPlatformText("-durations")) == 0) {
            metDurationsFlag = true;
            needToCaptureArgumentWhichIsAfterDurations = true;
        }
        else if (CrossPlatformCaseInsensitiveTextCompare(argv[i], CrossPlatformText("-fps")) == 0) {
            metFPSFlag = true;
        } else if (needToCaptureArgumentWhichIsAfterDurations) {
            argumentWhichIsAfterDurations = argv[i];
            needToCaptureArgumentWhichIsAfterDurations = false;
        } else {
            unparsedArgs.push_back(argv[i]);
        }
    }
    if (metFFlag) {
        if (unparsedArgs.size() != 1) {
            CrossPlatformCerr << CrossPlatformText("Filename or file path must be provided with -f option.\n");
            return -1;
        }
        filename = unparsedArgs.front();
        FILE* file = nullptr;
        if (!crossPlatformOpenFile(&file, filename)) {
            exit(-1);
        }
        int err;
        if (!metUFlag) {
            err = reportGIFDurationDurationsFormat(file);
        } else {
            err = reportGIFDuration(file);
        }
        if (err != 0) {
            CrossPlatformCerr << CrossPlatformText("Reading failed. Invalid GIF format.\n");
            fclose(file);
            exit(-1);
        }
        fclose(file);
        if (metUFlag) {
            CrossPlatformCout << CrossPlatformText("Finished successfully.\n");
        }
        return 0;
    } else if (!metDurationFlag && !metDurationsFlag && !metFPSFlag) {
        CrossPlatformCerr << CrossPlatformText("Must provide at least either -duration, -durations or -fps. Add --help or /? option for help.\n");
        return -1;
    } else if ((unsigned int)metDurationFlag + (unsigned int)metDurationsFlag + (unsigned int)metFPSFlag > 1) {
        CrossPlatformCerr << CrossPlatformText("Must provide only one of either -duration, -durations or -fps. Add --help or /? option for help.\n");
        return -1;
    } else if (metDurationsFlag && needToCaptureArgumentWhichIsAfterDurations) {
        CrossPlatformCerr << CrossPlatformText("A filename or filepath to the durations text file must be provided after a -durations option. Add --help or /? option for help.\n");
        return -1;
    } else if (metDurationsFlag && argumentWhichIsAfterDurations.empty()) {
        CrossPlatformCerr << CrossPlatformText("Path to the durations text file (after -durations option) is empty. Add --help or /? option for help.\n");
        return -1;
    } else if (metDurationsFlag) {
        if (unparsedArgs.size() != 1) {
            CrossPlatformCerr << CrossPlatformText("Can't understand where the filename or file path to the GIF file is - there are some unparsed arguments. Add --help or /? option for help.\n");
            return -1;
        }
        FILE* file = nullptr;
        if (!crossPlatformOpenFile(&file, unparsedArgs.front().c_str())) {
            exit(-1);
        }
        FILE* durationsFile = nullptr;
        if (!crossPlatformOpenFile(&durationsFile, argumentWhichIsAfterDurations.c_str())) {
            fclose(file);
            exit(-1);
        }
        struct GIFDuration_response response = changeGIFDurationFile(file, durationsFile);
        if (response.error != 0) {
            CrossPlatformCerr << CrossPlatformText("Operation failed.\n");
            if (response.modifications_count != 0) {
                CrossPlatformCerr << CrossPlatformText("Note: File could have been modified partially.\n");
            }
            fclose(file);
            fclose(durationsFile);
            exit(-1);
        }
        if (response.modifications_count == 0) {
            CrossPlatformCout << CrossPlatformText("Nothing modified.\n");
        }
        else {
            CrossPlatformCout << CrossPlatformText("Modified successfully.\n");
        }
        fclose(file);
        fclose(durationsFile);
        return 0;
    } else {
        if (unparsedArgs.size() != 3) {
            CrossPlatformCerr << CrossPlatformText("Invalid number of arguments. Add --help or /? option for help.\n");
            exit(-1);
        }
        int startInt = 0;
        int endInt = 0;
        bool startEndParsedSuccessfully = false;
        for (auto it = unparsedArgs.begin(); it != unparsedArgs.end(); ++it) {
            int pos = findChar(*it, CrossPlatformText('-'));
            if (pos != -1) {
                std::vector<CrossPlatformString> parts = split(*it, CrossPlatformText('-'));
                bool successParsing = true;
                if (parts.size() != 2) {
                    continue;
                }
                if (!parseInteger(parts[0], startInt)) {
                    continue;
                }
                if (!parseInteger(parts[1], endInt)) {
                    continue;
                }
                startEndParsedSuccessfully = true;
                unparsedArgs.erase(it);
                break;
            }
        }
        if (!startEndParsedSuccessfully) {
            CrossPlatformCerr << CrossPlatformText("Failed to parse start-end frame range. Add --help or /? option for help.\n");
            exit(-1);
        }
        if (startInt < 0 || endInt < 0 || endInt < startInt) {
            CrossPlatformCerr << CrossPlatformText("Parsed start-end frame range is invalid. Add --help or /? option for help.\n");
            CrossPlatformCerr << CrossPlatformText("Start: ") << startInt << CrossPlatformText("; End: ") << endInt << std::endl;
            exit(-1);
        }
        const size_t start = startInt;
        const size_t end = endInt;

        int valueToSet = 0;
        bool parsedValueToSet = false;
        for (auto it = unparsedArgs.begin(); it != unparsedArgs.end(); ++it) {
            if (!parseInteger(*it, valueToSet)) {
                continue;
            }
            parsedValueToSet = true;
            unparsedArgs.erase(it);
            break;
        }
        if (!parsedValueToSet) {
            if (metDurationFlag) {
                CrossPlatformCerr << CrossPlatformText("Failed to parse the value for the -duration option. Add --help or /? option for help.\n");
            } else if (metFPSFlag) {
                CrossPlatformCerr << CrossPlatformText("Failed to parse the value for the -fps option. Add --help or /? option for help.\n");
            } else {
                CrossPlatformCerr << CrossPlatformText("Unknown error related to parsing the value to set to. Add --help or /? option for help.\n");
            }
            exit(-1);
        }
        if (metFPSFlag) {
            valueToSet = 1000 / valueToSet;
        }
        FILE* file = nullptr;
        if (!crossPlatformOpenFile(&file, unparsedArgs.front())) {
            exit(-1);
        }
        struct GIFDuration_response response = changeGIFDurationRange(file, start, end, valueToSet);
        int returnCode = 0;
        if (response.error != 0) {
            CrossPlatformCerr << CrossPlatformText("Operation failed. Invalid GIF format. File could have been modified partially.\n");
            returnCode = -1;
        } else
        if (response.frame_count - 1 < start) {
            CrossPlatformCerr << CrossPlatformText("Input range outside of GIF length-1 (") << start << CrossPlatformText(" greater than ")
                << (int)(response.frame_count - 1) << CrossPlatformText("). File could have been modified partially.\n");
            returnCode = -1;
        } else
        if (response.frame_count - 1 < end) {
            CrossPlatformCerr << CrossPlatformText("Input range outside of GIF length-1 (") << end
                << CrossPlatformText(" greater than ") << (int)(response.frame_count - 1) << CrossPlatformText("). File could have been modified partially.\n");
            returnCode = -1;
        }
        if (response.modifications_count == 0) {
            CrossPlatformCout << CrossPlatformText("Nothing modified.\n");
        }
        else {
            CrossPlatformCout << CrossPlatformText("Modified successfully.\n");
        }
        fclose(file);
        return returnCode;
    }
    return 0;
}
