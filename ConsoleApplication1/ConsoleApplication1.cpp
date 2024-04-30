#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <locale.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

// Модель
class Node {
public:
    explicit Node(std::string name, bool isDirectory) : name_(std::move(name)), isDirectory_(isDirectory) {}

    void addChild(std::shared_ptr<Node> child) {
        children_.push_back(child);
    }

    const std::vector<std::shared_ptr<Node>>& getChildren() const {
        return children_;
    }

    const std::string& getName() const {
        return name_;
    }

    bool isDirectory() const {
        return isDirectory_;
    }

private:
    std::string name_;
    bool isDirectory_;
    std::vector<std::shared_ptr<Node>> children_;
};


// Представление
class TreeView {
public:

    void show(std::shared_ptr<Node> root) {
        displayNode(root, 0);
    }

    void showFiles(const std::vector<std::string>& files) {
        std::cout << "Найденные файлы:" << std::endl;
        for (const auto& file : files) {
            std::cout << file << std::endl;
        }
    }

    

    char getMenuChoice() {
        std::cout << "Меню:\n"
            << "1. Показать дерево\n"
            << "2. Поиск файла\n"
            << "3. Создать каталог\n"
            << "4. Создать файл\n"
            << "5. Удалить файл или каталог\n"
            << "6. Выход\n"
            << "Выберите: ";
        char choice;
        std::cin >> choice;
        return choice;
    }

private:
    void displayNode(std::shared_ptr<Node> node, int depth) {
        for (int i = 0; i < depth; ++i)
            std::cout << "  ";

        if (node->isDirectory())
            std::cout << "+ ";
        else
            std::cout << "- ";

        std::cout << node->getName() << std::endl;

        for (const auto& child : node->getChildren())
            displayNode(child, depth + 1);
    }
};

std::shared_ptr<Node> buildFileSystemTree(const std::string& path) {
    if (!fs::is_directory(path)) {
        throw std::runtime_error("Этот путь не существует или не является каталогом.");
    }

    auto root = std::make_shared<Node>(path, true);

    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_directory(entry.status())) {
            auto node = buildFileSystemTree(entry.path().string());
            root->addChild(node);
        }
        else {
            auto fileName = entry.path().filename().string();
            auto node = std::make_shared<Node>(fileName, false);
            root->addChild(node);
        }
    }

    return root;
}

// Контроллер
class TreeController {
public:
    TreeController(std::shared_ptr<Node> root, TreeView& view)
        : root_(root), view_(view) {}

    void processUserInput(const std::string& path) {
        char choice;
        do {
            choice = view_.getMenuChoice();
            switch (choice) {
            case '1':
                view_.show(root_);
                break;
            case '2':
                searchFile();
                break;
            case '3':
                createDirectory(path); 
                break;
            case '4':
                createFile(path);
                break;
            case '5':
                deleteFileOrDirectory(path);
                break;
            case '6':
                std::cout << "Выход...\n";
                break;
            default:
                std::cout << "Неверный выбор!\n";
            }
        } while (choice != '6');
    }

private:
    std::shared_ptr<Node> root_;
    TreeView& view_;

    void searchFile() {
        std::string fileName;
        std::cout << "Введите имя файла для поиска: ";
        std::cin >> fileName;
        std::vector<std::string> foundFiles;
        search(root_, fileName, foundFiles);
        view_.showFiles(foundFiles);
    }

    void search(std::shared_ptr<Node> node, const std::string& fileName, std::vector<std::string>& foundFiles) {
        for (const auto& child : node->getChildren()) {
            if (child->getName().find(fileName) != std::string::npos && !child->isDirectory()) {
                foundFiles.push_back(child->getName());
            }
            if (child->isDirectory()) {
                search(child, fileName, foundFiles);
            }
        }
    }

    void createDirectory(const std::string& path) {
        std::string dirName;
        std::cout << "Введите имя каталога для создания: ";
        std::cin >> dirName;
        fs::path dirPath(path);
        dirPath /= dirName;
        if (fs::create_directory(dirPath)) {
            std::cout << "Каталог успешно создан.\n";
            root_ = buildFileSystemTree(path);
        }
        else {
            std::cout << "Не удалось создать каталог.\n";
        }
    }
    void createFile(const std::string& path) {
        std::string fileName;
        std::cout << "Введите имя файла с расширением: ";
        std::cin >> fileName;
        fs::path filePath(path);
        filePath /= fileName;
        if (fs::exists(filePath)) {
            std::cout << "Файл уже существует.\n";
            return;
        }
        std::ofstream file(filePath.string());
        if (file.is_open()) {
            std::cout << "Файл успешно создан.\n";
            file.close();
            root_ = buildFileSystemTree(path);
        }
        else {
            std::cout << "Не удалось создать файл.\n";
        }
    }


    void deleteFileOrDirectory(const std::string& path) {
        std::string fileName;
        std::cout << "Введите имя файла или каталога для удаления: ";
        std::cin >> fileName;

        fs::path dirPath(path);
        dirPath /= fileName;

        if (fs::exists(dirPath)) {
            if (fs::is_directory(dirPath)) {
                if (fs::remove_all(dirPath)) {
                    std::cout << "Каталог успешно удален.\n";
                }
                else {
                    std::cout << "Не удалось удалить каталог.\n";
                }
            }
            else {
                if (fs::remove(dirPath)) {
                    std::cout << "Файл успешно удален.\n";
                }
                else {
                    std::cout << "Не удалось удалить файл.\n";
                }
            }
        }
        else {
            std::cout << "Файл или каталог не существует.\n";
        }
    }





};

int main() {
    setlocale(LC_ALL, "Russian");

    std::string path;
    std::cout << "Введите путь: ";
    std::cin >> path;

    TreeView view;
    std::shared_ptr<Node> root;
    try {
        root = buildFileSystemTree(path);
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    TreeController controller(root, view);

    controller.processUserInput(path);

    return 0;
}


