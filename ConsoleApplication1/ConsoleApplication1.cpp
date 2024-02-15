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

    void showTree() {
        view_.show(root_);
    }

private:
    std::shared_ptr<Node> root_;
    TreeView& view_;
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
    std::getline(std::cin, path);

    // Создание представления и контроллера
    TreeView view;
    auto root = buildFileSystemTree(path);
    TreeController controller(root, view);

    // Отображение дерева файловой системы
    controller.showTree();

    return 0;
}
