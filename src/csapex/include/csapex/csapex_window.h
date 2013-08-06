#ifndef EVALUATION_WINDOW_H
#define EVALUATION_WINDOW_H

/// SYSTEM
#include <QMainWindow>
#include <QTimer>

namespace Ui
{
class EvaluationWindow;
}

namespace csapex
{

class Designer;

/**
 * @brief The EvaluationWindow class provides the window for the evaluator program
 */
class EvaluationWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief EvaluationWindow
     * @param parent
     */
    explicit EvaluationWindow(QWidget* parent = 0);

    void showMenu();
    void closeEvent(QCloseEvent* event);
    void paintEvent(QPaintEvent * e);

    Designer* getDesigner();

private Q_SLOTS:
    void updateMenu();
    void updateTitle();
    void updateLog();
    void hideLog();
    void scrollDownLog();
    void init();

public Q_SLOTS:
    void start();
    void showStatusMessage(const std::string& msg);

Q_SIGNALS:
    void initialize();

private:
    Ui::EvaluationWindow* ui;

    QTimer timer;

    bool init_;
};

} /// NAMESPACE

#endif // EVALUATION_WINDOW_H