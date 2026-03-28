#include <QtWidgets>
#include <cstdlib>

class OllamaManager : public QWidget {
public:
    OllamaManager() {
        auto *layout = new QVBoxLayout(this);
        setWindowTitle("Ollama ROCm Manager");
        setMinimumWidth(400);
        auto *groupDocker = new QGroupBox("Container Management");
        auto *dockerLayout = new QHBoxLayout(groupDocker);
        auto *btnStart = new QPushButton("Start Container");
        auto *btnStop = new QPushButton("Stop Container");
        dockerLayout->addWidget(btnStart);
        dockerLayout->addWidget(btnStop);
        layout->addWidget(groupDocker);
        auto *groupModels = new QGroupBox("Model Operations");
        auto *modelLayout = new QVBoxLayout(groupModels);
        modelInput = new QLineEdit();
        modelInput->setPlaceholderText("Model name (e.g., llama3)");
        auto *btnDownload = new QPushButton("Download / Pull Model");
        auto *btnUnload = new QPushButton("Unload All Models from RAM");
        auto *btnDelete = new QPushButton("Delete Model");
        modelLayout->addWidget(modelInput);
        modelLayout->addWidget(btnDownload);
        modelLayout->addWidget(btnUnload);
        modelLayout->addWidget(btnDelete);
        layout->addWidget(groupModels);
        logOutput = new QTextEdit();
        logOutput->setReadOnly(true);
        layout->addWidget(logOutput);
        connect(btnStart, &QPushButton::clicked, [this]() { runCmd("docker compose up -d"); });
        connect(btnStop, &QPushButton::clicked, [this]() { runCmd("docker compose stop"); });
        connect(btnDownload, &QPushButton::clicked, [this]() {
    QString model = modelInput->text();
    if(model.isEmpty()) return;
    QString cmd = "gnome-terminal -- bash -c \"docker exec -it ollama-rocm ollama pull " + model + "; exec bash\"";
    logOutput->append("Starting download in new terminal: " + model);
    std::system(cmd.toStdString().c_str());
});
        connect(btnDelete, &QPushButton::clicked, [this]() { runCmd("docker exec ollama-rocm ollama rm " + modelInput->text()); });
        connect(btnUnload, &QPushButton::clicked, this, &OllamaManager::unloadModels);
    }

private:
    QLineEdit *modelInput;
    QTextEdit *logOutput;

    void runCmd(QString cmd) {
        logOutput->append("Executing: " + cmd);
        int result = std::system(cmd.toStdString().c_str());
        logOutput->append(result == 0 ? "Success" : "Error occurred");
    }

    void unloadModels() {
    QString model = modelInput->text();
    if (model.isEmpty()) {
        logOutput->append("Error: Enter model name to unload (e.g. llama3).");
        return;
    }
    QString cmd = "curl -s -X POST http://localhost:11434/api/generate -d '{\"model\": \"" + model + "\", \"keep_alive\": 0}'";
    runCmd(cmd);
    logOutput->append("Unload signal sent for: " + model);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    OllamaManager window;
    window.show();
    return app.exec();
}
