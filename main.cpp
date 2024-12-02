#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <ctime>

time_t now = time(0);
char* dt = ctime(&now);

class File {
public:
    virtual void displayContents() const = 0;
    virtual std::string getName() const = 0;
    virtual std::vector<int> getBytes() const = 0;
    virtual bool isDirectory() const = 0;
    virtual bool isSoftLinkedFile() const = 0;
    virtual File* getFile(const std::string& fileName) const = 0;
    virtual void removeFile(const std::string& fileName) = 0;
    virtual void addFile(File* file) = 0;
    virtual ~File() {}
};

class RegularFile : public File {
private:
    std::string name;
    std::vector<int> content;

public:
    RegularFile(const std::string& name, const std::vector<int>& content)
        : name(name), content(content) {}

    void displayContents() const override {
        for (int byte : content) {
            std::cout << byte;
        }
        std::cout << std::endl;
    }

    std::string getName() const override {
        return name;
    }

    std::vector<int> getBytes() const override {
        return content;
    }

    bool isDirectory() const override {
        return false;
    }

    bool isSoftLinkedFile() const override {
        return false;
    }

    File* getFile(const std::string& fileName) const override {
        if (fileName == name) {
            return const_cast<RegularFile*>(this);
        }
        return nullptr;
    }

    void removeFile(const std::string& fileName) override {}

    void addFile(File* file) override {}
};

class Directory : public File {
private:
    std::string name;
    std::vector<File*> contents;
    Directory* parent;
    std::size_t totalSize;

public:
    Directory(const std::string& name, Directory* parent = nullptr) : name(name), parent(parent), totalSize(0) {}

    void displayContents() const override {
        for (File* file : contents) {
            std::cout << file->getName() << " ";
        }
        std::cout << std::endl;
    }

    std::string getName() const override {
        return name;
    }

    std::vector<int> getBytes() const override {
        return {};
    }

    bool isDirectory() const override {
        return true;
    }

    bool isSoftLinkedFile() const override {
        return false;
    }
  const std::vector<File*>& getFiles() const {
        return contents;
    }
    File* getFile(const std::string& fileName) const override {
        for (File* file : contents) {
            if (file->getName() == fileName) {
                return file;
            }
        }
        return nullptr;
    }

    void removeFile(const std::string& fileName) override {
        auto it = std::remove_if(contents.begin(), contents.end(),
                                 [fileName](File* file) { return file->getName() == fileName; });

        if (it != contents.end()) {
            totalSize -= (*it)->getBytes().size();
            delete *it;
            contents.erase(it, contents.end());
        }
    }

    void addFile(File* file) override {
        if (totalSize + file->getBytes().size() <= 10 * 1024 * 1024) {
            contents.push_back(file);
            totalSize += file->getBytes().size();
        } else {
            std::cout << "Error: Disk space limit exceeded." << std::endl;
        }
    }

    Directory* getParent() const {
    if (parent && parent->getName() != "root") {
        return parent;
    } else {
        return nullptr;
    }
}

    ~Directory() {
        for (File* file : contents) {
            delete file;
        }
    }
};

class SoftLinkedFile : public File {
private:
    std::string name;
    File* linkedFile;

public:
    SoftLinkedFile(const std::string& name, File* linkedFile)
        : name(name), linkedFile(linkedFile) {}

    void displayContents() const override {
        linkedFile->displayContents();
    }

    std::string getName() const override {
        return name;
    }

    std::vector<int> getBytes() const override {
        return linkedFile->getBytes();
    }

    bool isDirectory() const override {
        return linkedFile->isDirectory();
    }

    bool isSoftLinkedFile() const override {
        return true;
    }

    File* getFile(const std::string& fileName) const override {
        if (fileName == name) {
            return const_cast<SoftLinkedFile*>(this);
        }
        return nullptr;
    }

    void removeFile(const std::string& fileName) override {}

    void addFile(File* file) override {}
};

class SimpleShell {
private:
    Directory* currentDir;

public:
    SimpleShell(Directory* root) : currentDir(root) {}

    void executeCommand(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        if (cmd == "ls") {
            currentDir->displayContents();
            std::cout << dt << std::endl << ">";
        } else if (cmd == "mkdir") {
            std::string dirName;
            iss >> dirName;
            Directory* newDir = new Directory(dirName);
            currentDir->addFile(newDir);
            std::cout << dt << std::endl << ">";
        } else if (cmd == "rm") {
            std::string fileName;
            iss >> fileName;
            currentDir->removeFile(fileName);
            std::cout << dt << std::endl << ">";
        } else if (cmd == "cp") {
            std::string sourceFileName, destFileName;
            iss >> sourceFileName >> destFileName;
            File* sourceFile = currentDir->getFile(sourceFileName);
            if (sourceFile) {
                File* copyFile = nullptr;
                if (sourceFile->isDirectory()) {
                    std::cout << "Error: Copying directories is not supported." << std::endl;
                } else {
                    copyFile = new RegularFile(destFileName, sourceFile->getBytes());
                }

                if (copyFile) {
                    currentDir->addFile(copyFile);
                }
            } else {
                std::cout << "Error: Source file not found." << std::endl;
            }
            std::cout << dt << std::endl << ">";
        } else if (cmd == "link") {
            std::string sourceFileName, destFileName;
            iss >> sourceFileName >> destFileName;
            File* sourceFile = currentDir->getFile(sourceFileName);
            if (sourceFile) {
                File* linkFile = new SoftLinkedFile(destFileName, sourceFile);
                currentDir->addFile(linkFile);
            } else {
                std::cout << "Error: Source file not found." << std::endl;
            }
            std::cout << dt << std::endl << ">";
        } else if (cmd == "cd") {
            std::string path;
            iss >> path;

          if (path == "..") {
    if (currentDir->getName() != "root") {
        Directory* parentDir = currentDir->getParent();
        if (parentDir) {
            currentDir = parentDir;
        } else {
            std::cout << "Error: Unable to determine parent directory." << std::endl;
        }
    } else {
        std::cout << "Error: Already at the root directory." << std::endl;
    }
} else if (path == ".") {
                // Do nothing for the current directory
            } else {
                File* newDir = currentDir->getFile(path);
                if (newDir && newDir->isDirectory()) {
                    currentDir = dynamic_cast<Directory*>(newDir);
                } else {
                    std::cout << "Error: Directory not found." << std::endl;
                }
            }
            std::cout << dt << std::endl << ">";
        } else if (cmd == "cat") {
            std::string fileName;
            iss >> fileName;
            File* file = currentDir->getFile(fileName);
            if (file) {
                file->displayContents();
            } else {
                std::cout << "Error: File not found." << std::endl;
            }
            std::cout << dt << std::endl << ">";
        } else if (cmd == "touch") {
            std::string fileName;
            iss >> fileName;
            RegularFile* newFile = new RegularFile(fileName, {});
            currentDir->addFile(newFile);
            std::cout << dt << std::endl << ">";
        } else {
            std::cout << "Error: Unknown command." << std::endl;
            std::cout << dt << std::endl << ">";
        }
    }
};

void serializeFile(std::ofstream& outFile, const File* file) {
    const char file_type = file->isDirectory() ? 'D' : (file->isSoftLinkedFile() ? 'L' : 'R');
    outFile.write(&file_type, sizeof(char));

    const std::string name = file->getName();
    const size_t nameLength = name.size();
    outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
    outFile.write(name.c_str(), nameLength);

    if (!file->isDirectory()) {
        const std::vector<int> bytes = file->getBytes();
        const size_t numBytes = bytes.size();
        outFile.write(reinterpret_cast<const char*>(&numBytes), sizeof(size_t));
        outFile.write(reinterpret_cast<const char*>(bytes.data()), numBytes * sizeof(int));
    }
 if (file->isDirectory()) {
        const Directory* dir = static_cast<const Directory*>(file);
        const size_t numContents = dir->getFiles().size();
        outFile.write(reinterpret_cast<const char*>(&numContents), sizeof(size_t));
        for (const File* subFile : dir->getFiles()) {
            serializeFile(outFile, subFile);
        }
    }
}

File* deserializeFile(std::ifstream& inFile) {
    char file_type;
    inFile.read(&file_type, sizeof(char));

    size_t nameLength;
    inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
    std::string name(nameLength, '\0');
    inFile.read(&name[0], nameLength);

    if (file_type == 'R') {
        size_t numBytes;
        inFile.read(reinterpret_cast<char*>(&numBytes), sizeof(size_t));
        std::vector<int> bytes(numBytes);
        inFile.read(reinterpret_cast<char*>(bytes.data()), numBytes * sizeof(int));
        return new RegularFile(name, bytes);
    } else if (file_type == 'L') {
        File* linkedFile = deserializeFile(inFile);
        return new SoftLinkedFile(name, linkedFile);
    } else if (file_type == 'D') {
        Directory* dir = new Directory(name);
        size_t numContents;
        inFile.read(reinterpret_cast<char*>(&numContents), sizeof(size_t));
        for (size_t i = 0; i < numContents; ++i) {
            File* subFile = deserializeFile(inFile);
            dir->addFile(subFile);
        }
        return dir;
    } else {
        std::cerr << "Error: Unknown file type during deserialization." << std::endl;
        return nullptr;
    }
}

void serializeFileSystem(Directory* rootDir) {
    std::ofstream outFile("filesystem_state.txt", std::ios::binary | std::ios::trunc);

    if (outFile.is_open()) {
        serializeFile(outFile, rootDir);
        outFile.close();
    } else {
        std::cerr << "Error: Unable to open the file for serialization." << std::endl;
    }
}

Directory* deserializeFileSystem() {
    std::ifstream inFile("filesystem_state.txt", std::ios::binary);

    if (inFile.is_open()) {
        File* rootFile = deserializeFile(inFile);
        if (rootFile && rootFile->isDirectory()) {
            return static_cast<Directory*>(rootFile);
        } else {
            std::cerr << "Error: Invalid file system structure during deserialization." << std::endl;
            return new Directory("root");
        }
    } else {
        std::cerr << "Error: Unable to open the file for deserialization. Creating a new file system." << std::endl;
        return new Directory("root");
    }
}

int main() {
    Directory* rootDir = deserializeFileSystem();

    SimpleShell shell(rootDir);

    std::cout << "myShell" << std::endl;

    while (true) {
        std::cout << "> ";
        std::string userInput;
        std::getline(std::cin, userInput);

        if (userInput == "exit") {
            serializeFileSystem(rootDir);
            break;
        }

        shell.executeCommand(userInput);
    }

    delete rootDir;

    return 0;
}
