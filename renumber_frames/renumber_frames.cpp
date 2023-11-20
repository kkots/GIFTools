
#include <iostream>
#include <string>
#ifndef FOR_LINUX
#include <Windows.h>
#include "WinError.h"
#else
#include <fstream>
#include <string.h>
#include <stdio.h>
#endif
#include "CrossPlatformDefs.h"
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
    if (!*file) return false;
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

CrossPlatformString repeatChar(CrossPlatformChar c, int n) {
    return CrossPlatformString(n, c);
}

bool crossPlatformMoveFile(const CrossPlatformString& source, const CrossPlatformString& dest) {
    #ifndef FOR_LINUX
    if (!MoveFileExW(source.c_str(), dest.c_str(), MOVEFILE_WRITE_THROUGH)) {
        WinError winErr;
        CrossPlatformCerr << "Error moving file from " << source.c_str() << " to " << dest.c_str() << ": " << winErr.getMessage() << std::endl;
        return false;
    }
    #else
    int errCode = rename(source.c_str(), dest.c_str());

    if (errCode) {
        CrossPlatformCerr << "Error moving file from " << source.c_str() << " to " << dest.c_str() << ": " << strerror(errCode) << std::endl;
        return false;
    }
    #endif
    return true;
}

CrossPlatformString numberToStringAndPadArena;

CrossPlatformString& numberToStringAndPad(int numberToBeConverted, size_t totalCountReqChars) {
    numberToStringAndPadArena.reserve(totalCountReqChars);
    numberToStringAndPadArena = CrossPlatformNumberToString(numberToBeConverted);
    while (numberToStringAndPadArena.size() < totalCountReqChars) {
        numberToStringAndPadArena.insert(numberToStringAndPadArena.begin(), CrossPlatformText('0'));
    }
    return numberToStringAndPadArena;
}

#define PARAMETERS_FORMAT_HELP CrossPlatformText("1 - input/output file path (files will be renamed) points to files with names like")\
    CrossPlatformText(" image1.png, image2.png, image3.png, where the 1, 2, 3, etc part is replaced with a % sign.\n")\
    CrossPlatformText("Use multiple % signs if you want the number to be 0-padded on the left.\n")\
	CrossPlatformText("2 - frame range in format 0-20. This specifies the range of frames to move.\n")\
	CrossPlatformText("3 - destination frame number to move the frames to.\n")


int CrossPlatformMainName(int argc, CrossPlatformChar* argv[], CrossPlatformChar* envp[])
{
    if (argc == 2 && (
        CrossPlatformCaseInsensitiveTextCompare(argv[1], CrossPlatformText("-help")) == 0
        || CrossPlatformCaseInsensitiveTextCompare(argv[1], CrossPlatformText("--help")) == 0
        || CrossPlatformCaseInsensitiveTextCompare(argv[1], CrossPlatformText("/?")) == 0
        )) {
        CrossPlatformCout << CrossPlatformText("The program shuffles files that have numbers in their names. Expects arguments:\n") PARAMETERS_FORMAT_HELP;
        exit(0);
    }

    if (argc != 4) {
        CrossPlatformCerr << CrossPlatformText("Wrong number of argument. Use --help or /? option for help.\n");
        exit(-1);
    }

    CrossPlatformString path = argv[1];
    if (path.empty()) {
        CrossPlatformCerr << CrossPlatformText("The provided file path is empty. Use --help or /? option for help.\n");
        exit(-1);
    }

    int pos = findChar(path, CrossPlatformText('%'));
    if (pos == -1) {
        CrossPlatformCerr << CrossPlatformText("Error: provided file does not contain a % character which is supposed to mean the number part of the file name.")
            CrossPlatformText(" Use --help or /? option for help.\n");
        exit(-1);
    }

    size_t numberOfPercentSigns = 1;
    int posPtr = pos + 1;
    while ((size_t)posPtr < path.size()) {
        if (path[posPtr] == CrossPlatformText('%')) {
            ++numberOfPercentSigns;
        } else {
            break;
        }
        ++posPtr;
    }

    CrossPlatformString pathBeforePercents = CrossPlatformString{ path.begin(), path.begin() + pos };
    CrossPlatformString pathAfterPercents = CrossPlatformString{ path.begin() + pos + numberOfPercentSigns, path.end() };

    pos = findChar(argv[2], CrossPlatformText('-'));
    if (pos == -1) {
        CrossPlatformCerr << CrossPlatformText("Error: provided frame range does not contain a - character which is supposed to separate the start")
            CrossPlatformText(" and end frame range values. Use --help or /? option for help.\n");
        exit(-1);
    }

    int start = 0;
    int end = 0;
    if (!parseInteger(CrossPlatformString{argv[2], (size_t)pos}, start)) {
        CrossPlatformCerr << CrossPlatformText("Error: failed to parse the starting frame of the frame range argument. Use --help or /? option for help.\n");
        exit(-1);
    }
    if (!parseInteger(CrossPlatformString{ argv[2] + pos + 1 }, end)) {
        CrossPlatformCerr << CrossPlatformText("Error: failed to parse the ending frame of the frame range argument. Use --help or /? option for help.\n");
        exit(-1);
    }
    if (start < 0 || end < 0 || end < start) {
        CrossPlatformCerr << CrossPlatformText("Error: the parsed frame range is invalid. Use --help or /? option for help.\n");
        CrossPlatformCerr << CrossPlatformText("Start: ") << start << CrossPlatformText("; End: ") << end << std::endl;
        exit(-1);
    }

    int dest = 0;
    if (!parseInteger(CrossPlatformString{ argv[3] }, dest)) {
        CrossPlatformCerr << CrossPlatformText("Error: failed to parse the destination frame argument. Use --help or /? option for help.\n");
        exit(-1);
    }
    if (dest < 0) {
        CrossPlatformCerr << CrossPlatformText("Error: the destination frame argument cannot be less than 0. Use --help or /? option for help.\n");
        exit(-1);
    }

    if (dest == start) {
        CrossPlatformCout << CrossPlatformText("There's nothing to move, destination is equal to start.\n");
        exit(0);
    }

    if (dest < start) {
        int finalIndex = dest + end - start;
        if (finalIndex >= start) finalIndex = start - 1;
        CrossPlatformString destPath;
        for (int i = dest; i <= finalIndex; ++i) {
            destPath = pathBeforePercents;
            destPath += numberToStringAndPad(i, numberOfPercentSigns);
            destPath += pathAfterPercents;
            if (fileExists(destPath)) {
                CrossPlatformCerr << CrossPlatformText("Cannot perform operation because file ")
                    << destPath.c_str() << CrossPlatformText(" exists, is in the way and would be overwritten by the renames. Nothing got moved.\n");
                exit(-1);
            }
        }
        CrossPlatformString sourcePath;
        for (int i = start; i <= end; ++i) {
            sourcePath = pathBeforePercents;
            sourcePath += numberToStringAndPad(i, numberOfPercentSigns);
            sourcePath += pathAfterPercents;
            destPath = pathBeforePercents;
            destPath += numberToStringAndPad(dest, numberOfPercentSigns);
            destPath += pathAfterPercents;
            crossPlatformMoveFile(sourcePath, destPath);
            ++dest;
        }
    } else {
        int firstIndex = dest;
        if (firstIndex <= end) firstIndex = end + 1;
        const int lastIndex = dest + end - start;
        for (int i = firstIndex; i <= lastIndex; ++i) {
            CrossPlatformString destPath = pathBeforePercents + numberToStringAndPad(i, numberOfPercentSigns) + pathAfterPercents;
            if (fileExists(destPath)) {
                CrossPlatformCerr << CrossPlatformText("Cannot perform operation because file ")
                    << destPath.c_str() << CrossPlatformText(" exists, is in the way and would be overwritten by the renames. Nothing got moved.\n");
                exit(-1);
            }
        }
        CrossPlatformString sourcePath;
        CrossPlatformString destPath;
        dest = dest + end - start;
        for (int i = end; i >= start; --i) {
            sourcePath = pathBeforePercents;
            sourcePath += numberToStringAndPad(i, numberOfPercentSigns);
            sourcePath += pathAfterPercents;
            destPath = pathBeforePercents;
            destPath += numberToStringAndPad(dest, numberOfPercentSigns);
            destPath += pathAfterPercents;
            crossPlatformMoveFile(sourcePath, destPath);
            --dest;
        }
    }

    CrossPlatformCout << CrossPlatformText("Moved successfully.\n");
    return 0;
}