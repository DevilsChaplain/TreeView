#include <iostream>
#include <vector>
#include <memory>
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
        std::cout << "Found files:" << std::endl;
        for (const auto& file : files) {
            std::cout << file << std::endl;
        }
    }

    std::string getFileNameToDelete() {
        std::string fileName;
        std::cout << "Enter filename to delete: ";
        std::cin >> fileName;
        return fileName;
    }

    char getMenuChoice() {
        std::cout << "Menu:\n"
            << "1. Show tree\n"
            << "2. Search file\n"
            << "3. Exit\n"
            << "Enter choice: ";
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

// Контроллер
class TreeController {
public:
    TreeController(std::shared_ptr<Node> root, TreeView& view)
        : root_(root), view_(view) {}

    void processUserInput() {
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
                std::cout << "Exiting...\n";
                break;
            default:
                std::cout << "Invalid choice!\n";
            }
        } while (choice != '3');
    }

private:
    std::shared_ptr<Node> root_;
    TreeView& view_;

    void searchFile() {
        std::string fileName;
        std::cout << "Enter filename to search: ";
        std::cin >> fileName;
        std::vector<std::string> foundFiles;
        search(root_, fileName, foundFiles);
        view_.showFiles(foundFiles);
    }

    void search(std::shared_ptr<Node> node, const std::string& fileName, std::vector<std::string>& foundFiles) {
        for (const auto& child : node->getChildren()) {
            // Проверяем, содержит ли имя файла подстроку fileName
            if (child->getName().find(fileName) != std::string::npos && !child->isDirectory()) {
                foundFiles.push_back(child->getName());
            }
            if (child->isDirectory()) {
                search(child, fileName, foundFiles);
            }
        }
    }
};

// Создание дерева файловой системы
std::shared_ptr<Node> buildFileSystemTree(const std::string& path) {
    auto root = std::make_shared<Node>(path, true);

    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_directory(entry)) {
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
int main() {
    // Запрос пути у пользователя
    std::string path;
    std::cout << "Enter path: ";
    std::cin >> path;

    // Создание представления и контроллера
    TreeView view;
    auto root = buildFileSystemTree(path);
    TreeController controller(root, view);

    // Отображение меню и обработка действий пользователя
    controller.processUserInput();

    return 0;
}
