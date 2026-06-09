#ifndef GENERATORDIALOG_H
#define GENERATORDIALOG_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include "my/my_array.hpp"
#include "my/my_shared_ptr.hpp"
#include "my/my_random.hpp"
#include "lazy/LazySequence.hpp"

class ValueProvider;

namespace Ui {
class GeneratorDialog;
}

class GeneratorDialog : public QDialog {
    Q_OBJECT

public:
    explicit GeneratorDialog(QWidget *parent = nullptr);
    ~GeneratorDialog();

    SharedPtr<LazySequence<double>> getGeneratedSequence() const;

private slots:
    void updatePreview();

private:
    Ui::GeneratorDialog *ui;
    QChartView *chartView;
    mutable LCG rng;

    Array<SharedPtr<ValueProvider>> providers;
    int currentProvider;

    void initProviders();
    void updateProviderUI();
    void generatePreview(int maxPoints = 1000);
};

#endif