#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "my_shared_ptr.hpp"
#include "my_array.hpp"
#include "lazy/LazySequence.hpp"
#include <QtCharts/QChartView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_4_clicked();           // генератор
    void on_pushButton_5_clicked();           // создать последовательность
    void on_pushButton_6_clicked();           // Where
    void on_pushButton_7_clicked();           // Map
    void on_pushButton_8_clicked();           // Skip
    void on_pushButton_9_clicked();           // Concat
    void on_pushButton_10_clicked();          // Append
    void on_pushButton_11_clicked();          // Prepend
    void on_pushButton_12_clicked();          // InsertAt
    void on_pushButton_13_clicked();          // Extract selected rows
    void on_pushButton_left_clicked();
    void on_pushButton_right_clicked();
    void on_pushButtonUpdateHistogram_clicked();
    void on_tableWidget_cellChanged(int row, int column);

private:
    Ui::MainWindow *ui;
    SharedPtr<LazySequence<double>> currentSeq;
    Array<SharedPtr<LazySequence<double>>> history;
    size_t currentOffset = 0;
    static constexpr size_t DISPLAY_SIZE = 1000;

    void setCurrentSequence(SharedPtr<LazySequence<double>> newSeq, bool addToHistory = true);
    void updateInfoPanel();
    void updateTable();
    void logMessage(const QString& msg);
    void applyOperation(const QString& opName, SharedPtr<LazySequence<double>> newSeq, bool addToHistory = false);
    void replaceCurrentSequence(SharedPtr<LazySequence<double>> newSeq, const QString& opName, bool preserveOffset = true);
    SharedPtr<LazySequence<double>> selectSequenceFromHistory(const QString& title);
    void enableButtonsBasedOnSequence();
    void updateHistogram();
};

#endif
