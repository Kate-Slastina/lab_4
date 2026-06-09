#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "generatordialog.h"
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QCursor>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QDebug>
#include <cmath>
#include <ctime>
#include "my/my_random.hpp"
#include "my/my_distributions.hpp"

// ----- Вспомогательные функторы -----
struct ArithmeticGen {
    double first, step;
    int n;
    double operator()() { return first + step * n++; }
};

struct RandomGen {
    LCG rng;
    RandomGen(unsigned seed = static_cast<unsigned>(std::time(nullptr))) : rng(seed) {}
    double operator()() {
        UniformDist dist(0.0, 1.0);
        return dist(rng);
    }
};

struct ConstGen {
    double val;
    double operator()() { return val; }
};

struct FactorialGen {
    unsigned long long n = 1;
    double fact = 1.0;
    double operator()() {
        fact *= n;
        ++n;
        return fact;
    }
};

struct FibonacciGen {
    double a = 0.0, b = 1.0;
    double operator()() {
        double cur = a;
        double nxt = a + b;
        a = b;
        b = nxt;
        return cur;
    }
};

// ----- MainWindow -----
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), currentOffset(0) {
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(1);
    ui->tableWidget->setHorizontalHeaderLabels({"Значение"});
    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked |
                                     QAbstractItemView::EditKeyPressed);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(ui->tableWidget, &QTableWidget::cellChanged,
            this, &MainWindow::on_tableWidget_cellChanged);
    connect(ui->pushButtonUpdateHistogram, &QPushButton::clicked,
            this, &MainWindow::on_pushButtonUpdateHistogram_clicked);

    enableButtonsBasedOnSequence();
    updateHistogram();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::enableButtonsBasedOnSequence() {
    bool hasSeq = (currentSeq.get() != nullptr);
    ui->pushButton_10->setEnabled(hasSeq && currentSeq->IsFinite());
    ui->pushButton_9->setEnabled(hasSeq);
    ui->pushButton_6->setEnabled(hasSeq);
    ui->pushButton_7->setEnabled(hasSeq);
    ui->pushButton_11->setEnabled(hasSeq);
    ui->pushButton_12->setEnabled(hasSeq);
    ui->pushButton_13->setEnabled(hasSeq);
    ui->pushButton_8->setEnabled(hasSeq);
    bool showPageButtons = hasSeq && !currentSeq->IsFinite();
    ui->pushButton_left->setVisible(showPageButtons);
    ui->pushButton_right->setVisible(showPageButtons);
    if (showPageButtons) {
        ui->pushButton_left->setEnabled(currentOffset > 0);
        ui->pushButton_right->setEnabled(true);
    }
}

void MainWindow::on_pushButton_4_clicked() {
    GeneratorDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        auto newSeq = dlg.getGeneratedSequence();
        if (newSeq.get() != nullptr) {
            setCurrentSequence(newSeq, true);
            logMessage("Последовательность сгенерирована");
        } else {
            logMessage("Генерация не дала результата");
        }
    } else {
        logMessage("Генерация отменена");
    }
}

void MainWindow::on_pushButton_5_clicked() {
    QMenu menu;
    QAction* actNewFinite = menu.addAction("Создать конечную");
    QAction* actNewInfinite = menu.addAction("Создать бесконечную");
    QAction* actLoad = menu.addAction("Выбрать из созданных");
    QAction* selected = menu.exec(QCursor::pos());
    if (!selected) return;

    if (selected == actNewFinite) {
        bool ok;
        int count = QInputDialog::getInt(this, "Количество элементов", "Введите число элементов:", 10, 0, 10000000, 1, &ok);
        if (!ok || count == 0) return;
        QMessageBox::StandardButton method = QMessageBox::question(this, "Способ ввода", "Вводить элементы по одному?", QMessageBox::Yes | QMessageBox::No);
        Array<double> values;
        if (method == QMessageBox::Yes) {
            for (int i = 0; i < count; ++i) {
                double val = QInputDialog::getDouble(this, "Элемент", QString("Элемент %1:").arg(i), 0.0);
                values.push_back(val);
            }
        } else {
            double fillVal = QInputDialog::getDouble(this, "Заполнить", "Введите значение для всех элементов:", 0.0);
            for (int i = 0; i < count; ++i) values.push_back(fillVal);
        }
        auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(values.begin(), values.size()));
        setCurrentSequence(newSeq, true);
        logMessage(QString("Создана конечная последовательность из %1 элементов").arg(count));
    }
    else if (selected == actNewInfinite) {
        QMenu infMenu;
        QAction* actArithmetic = infMenu.addAction("Арифметическая прогрессия");
        QAction* actRandom = infMenu.addAction("Случайные числа (равномерное)");
        QAction* actConst = infMenu.addAction("Константная последовательность");
        QAction* actFactorial = infMenu.addAction("Факториал (n!)");
        QAction* actFibonacci = infMenu.addAction("Фибоначчи");
        QAction* infChoice = infMenu.exec(QCursor::pos());
        if (!infChoice) return;

        if (infChoice == actArithmetic) {
            bool ok;
            double first = QInputDialog::getDouble(this, "Арифметическая прогрессия", "Первый элемент:", 0.0, -1e9, 1e9, 2, &ok);
            if (!ok) return;
            double step = QInputDialog::getDouble(this, "Арифметическая прогрессия", "Шаг:", 1.0, -1e9, 1e9, 2, &ok);
            if (!ok) return;
            ArithmeticGen gen{first, step, 0};
            auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(Function<double()>(gen)));
            setCurrentSequence(newSeq, true);
            logMessage(QString("Создана бесконечная арифметическая прогрессия (a0=%1, d=%2)").arg(first).arg(step));
        }
        else if (infChoice == actRandom) {
            RandomGen gen;
            auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(Function<double()>(gen)));
            setCurrentSequence(newSeq, true);
            logMessage("Создана бесконечная последовательность случайных чисел [0,1)");
        }
        else if (infChoice == actConst) {
            bool ok;
            double value = QInputDialog::getDouble(this, "Константная последовательность", "Значение:", 0.0, -1e9, 1e9, 2, &ok);
            if (!ok) return;
            ConstGen gen{value};
            auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(Function<double()>(gen)));
            setCurrentSequence(newSeq, true);
            logMessage(QString("Создана бесконечная константная последовательность (значение %1)").arg(value));
        }
        else if (infChoice == actFactorial) {
            FactorialGen gen;
            auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(Function<double()>(gen)));
            setCurrentSequence(newSeq, true);
            logMessage("Создана бесконечная последовательность факториалов");
        }
        else if (infChoice == actFibonacci) {
            FibonacciGen gen;
            auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(Function<double()>(gen)));
            setCurrentSequence(newSeq, true);
            logMessage("Создана бесконечная последовательность Фибоначчи");
        }
    }
    else if (selected == actLoad) {
        auto seq = selectSequenceFromHistory("Выберите последовательность");
        if (seq.get() != nullptr) {
            setCurrentSequence(seq, true);
            logMessage("Загружена последовательность из истории");
        }
    }
}

void MainWindow::on_pushButton_6_clicked() {
    if (!currentSeq) {
        logMessage("Нет текущей последовательности");
        return;
    }

    auto pred = [](double x) -> bool {
        double intpart;
        if (std::modf(x, &intpart) != 0.0) return false;
        return static_cast<long long>(intpart) % 2 == 0;
    };

    if (!currentSeq->IsFinite()) {
        try {
            auto newSeq = currentSeq->Where(pred);
            // Для Where сбрасываем offset (начинаем с начала новой последовательности)
            replaceCurrentSequence(newSeq, "Where (чётность) для бесконечной", false);
        } catch (const EndOfSequence& e) {
            logMessage(QString("Фильтрация не удалась: %1").arg(e.what()));
        }
        return;
    }

    // Конечная последовательность
    Array<double> filtered;
    for (size_t i = 0; i < currentSeq->GetLength(); ++i) {
        double val = currentSeq->Get(i);
        if (pred(val)) filtered.push_back(val);
    }
    if (filtered.empty()) {
        QMessageBox::warning(this, "Предупреждение",
                             "В конечной последовательности не найдено ни одного целого чётного числа.\n"
                             "После фильтрации не останется элементов.");
        return;
    }
    auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(filtered.begin(), filtered.size()));
    replaceCurrentSequence(newSeq, "Where (чётность)", true);
}

void MainWindow::on_pushButton_7_clicked() {
    if (!currentSeq) return;
    auto mapper = [](double x) -> double { return x * 10.0; };
    if (!currentSeq->IsFinite()) {
        struct MapFunctor {
            SharedPtr<LazySequence<double>> seq;
            Function<double(double)> f;
            mutable size_t pos;
            double operator()() const { return f(seq->Get(pos++)); }
        };
        MapFunctor map{currentSeq, mapper, 0};
        auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(Function<double()>(map)));
        replaceCurrentSequence(newSeq, "Map (x*10) для бесконечной", true);
        return;
    }
    Array<double> mapped;
    for (size_t i = 0; i < currentSeq->GetLength(); ++i)
        mapped.push_back(mapper(currentSeq->Get(i)));
    auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(mapped.begin(), mapped.size()));
    replaceCurrentSequence(newSeq, "Map (x*10)", true);
}

void MainWindow::on_pushButton_8_clicked() {
    if (!currentSeq) return;
    bool ok;
    int count = QInputDialog::getInt(this, "Skip", "Сколько элементов пропустить?", 1, 0, 1000000, 1, &ok);
    if (!ok) return;
    try {
        auto newSeq = currentSeq->SkipFirst(static_cast<size_t>(count));
        applyOperation(QString("Skip (%1)").arg(count), newSeq, false);
    } catch (const std::exception& e) {
        logMessage("Ошибка Skip: " + QString(e.what()));
    }
}

void MainWindow::on_pushButton_9_clicked() {
    if (!currentSeq) return;
    auto other = selectSequenceFromHistory("Выберите последовательность для конкатенации");
    if (!other) {
        logMessage("Конкатенация отменена");
        return;
    }
    if (!currentSeq->IsFinite()) {
        logMessage("Конкатенация с бесконечной последовательностью: результат оставлен без изменений");
        return;
    }
    auto newSeq = currentSeq->Concat(other.get());
    replaceCurrentSequence(newSeq, "Concat", true);
}

void MainWindow::on_pushButton_10_clicked() {
    if (!currentSeq || !currentSeq->IsFinite()) {
        logMessage("Добавление в конец возможно только для конечной последовательности");
        return;
    }
    bool ok;
    double val = QInputDialog::getDouble(this, "Append", "Введите значение:", 0.0, -1e9, 1e9, 2, &ok);
    if (!ok) return;
    auto newSeq = currentSeq->Append(val);
    replaceCurrentSequence(newSeq, QString("Append(%1)").arg(val), true);
}

void MainWindow::on_pushButton_11_clicked() {
    if (!currentSeq) return;
    bool ok;
    double val = QInputDialog::getDouble(this, "Prepend", "Введите значение:", 0.0, -1e9, 1e9, 2, &ok);
    if (!ok) return;
    auto newSeq = currentSeq->Prepend(val);
    replaceCurrentSequence(newSeq, QString("Prepend(%1)").arg(val), true);
}

void MainWindow::on_pushButton_12_clicked() {
    if (!currentSeq) return;
    bool ok;
    int index = QInputDialog::getInt(this, "InsertAt", "Индекс для вставки:", 0, 0, 1000000, 1, &ok);
    if (!ok) return;
    double val = QInputDialog::getDouble(this, "InsertAt", "Значение:", 0.0, -1e9, 1e9, 2, &ok);
    if (!ok) return;
    try {
        auto newSeq = currentSeq->InsertAt(static_cast<size_t>(index), val);
        replaceCurrentSequence(newSeq, QString("InsertAt(%1, %2)").arg(index).arg(val), true);
    } catch (const IndexOutOfRange&) {
        logMessage("Ошибка: индекс превышает длину");
    }
}

void MainWindow::on_pushButton_13_clicked() {
    if (!currentSeq) return;
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        logMessage("Не выделено ни одной строки");
        return;
    }
    QSet<int> selectedRows;
    for (QTableWidgetItem* item : selectedItems) selectedRows.insert(item->row());
    QList<int> rows = selectedRows.values();
    std::sort(rows.begin(), rows.end());
    Array<double> values;
    for (int row : rows) {
        try {
            values.push_back(currentSeq->Get(static_cast<size_t>(row)));
        } catch (const std::exception& e) {
            logMessage("Ошибка чтения строки " + QString::number(row) + ": " + e.what());
            return;
        }
    }
    auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(values.begin(), values.size()));
    replaceCurrentSequence(newSeq, QString("Оставлены выделенные строки (%1)").arg(rows.size()), true);
}

void MainWindow::on_pushButton_left_clicked() {
    if (!currentSeq || currentSeq->IsFinite()) return;
    if (currentOffset > 0) {
        --currentOffset;
        updateTable();
        logMessage(QString("Диапазон: %1 – %2")
                       .arg(currentOffset * DISPLAY_SIZE)
                       .arg((currentOffset + 1) * DISPLAY_SIZE - 1));
        ui->pushButton_left->setEnabled(currentOffset > 0);
    }
}

void MainWindow::on_pushButton_right_clicked() {
    if (!currentSeq || currentSeq->IsFinite()) return;
    ++currentOffset;
    updateTable();
    logMessage(QString("Диапазон: %1 – %2")
                   .arg(currentOffset * DISPLAY_SIZE)
                   .arg((currentOffset + 1) * DISPLAY_SIZE - 1));
    ui->pushButton_left->setEnabled(true);
}

void MainWindow::on_pushButtonUpdateHistogram_clicked() {
    updateHistogram();
    logMessage("Гистограмма обновлена");
}

void MainWindow::on_tableWidget_cellChanged(int row, int column) {
    if (!currentSeq || !currentSeq->IsFinite()) {
        logMessage("Редактирование бесконечной последовательности не поддерживается");
        updateTable();
        return;
    }
    if (column != 0) return;

    QTableWidgetItem* item = ui->tableWidget->item(row, column);
    if (!item) return;

    bool ok;
    double newVal = item->text().toDouble(&ok);
    if (!ok) {
        updateTable();
        logMessage("Неверный формат числа");
        return;
    }

    size_t globalIndex = static_cast<size_t>(row);
    if (globalIndex >= currentSeq->GetLength()) {
        updateTable();
        return;
    }

    Array<double> values;
    for (size_t i = 0; i < currentSeq->GetLength(); ++i) {
        if (i == globalIndex)
            values.push_back(newVal);
        else
            values.push_back(currentSeq->Get(i));
    }
    auto newSeq = SharedPtr<LazySequence<double>>(new LazySequence<double>(values.begin(), values.size()));
    setCurrentSequence(newSeq, true);
    logMessage(QString("Элемент [%1] изменён на %2").arg(globalIndex).arg(newVal));
}

SharedPtr<LazySequence<double>> MainWindow::selectSequenceFromHistory(const QString& title) {
    if (history.empty()) {
        QMessageBox::information(this, "Нет данных", "Нет ранее созданных последовательностей.");
        return SharedPtr<LazySequence<double>>();
    }
    QStringList items;
    for (size_t i = 0; i < history.size(); ++i) {
        auto& seq = history[i];
        items << QString("Seq #%1: %2, длина %3").arg(i)
                     .arg(seq->IsFinite() ? "конечная" : "бесконечная")
                     .arg(seq->IsFinite() ? QString::number(seq->GetLength()) : "∞");
    }
    bool ok;
    QString chosen = QInputDialog::getItem(this, title, "Выберите последовательность:", items, 0, false, &ok);
    if (!ok || chosen.isEmpty()) return SharedPtr<LazySequence<double>>();
    int idx = items.indexOf(chosen);
    if (idx >= 0 && idx < static_cast<int>(history.size())) return history[idx];
    return SharedPtr<LazySequence<double>>();
}

void MainWindow::updateInfoPanel() {
    ui->listWidget->clear();
    if (!currentSeq) return;

    QString info;
    info += "Тип: " + QString(currentSeq->IsFinite() ? "конечная" : "бесконечная") + "\n";
    if (currentSeq->IsFinite()) {
        info += "Длина: " + QString::number(currentSeq->GetLength()) + "\n";
        info += "Кэшировано: " + QString::number(currentSeq->GetLength()) + "\n";
        info += "Память: ~" + QString::number(currentSeq->GetLength() * sizeof(double) / 1024.0 / 1024.0, 'f', 2) + " MB";
    } else {
        info += "Длина: бесконечность\n";
        info += "Диапазон: " + QString::number(currentOffset * DISPLAY_SIZE) + " – " +
                QString::number((currentOffset + 1) * DISPLAY_SIZE - 1) + "\n";
        info += "Кэшировано: ленивая генерация\n";
    }
    ui->listWidget->addItem(info);
}

void MainWindow::updateTable() {
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();

    if (!currentSeq) {
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->blockSignals(false);
        return;
    }

    size_t start = 0;
    size_t rows = 0;
    if (currentSeq->IsFinite()) {
        start = 0;
        rows = std::min(currentSeq->GetLength(), DISPLAY_SIZE);
        currentOffset = 0;
    } else {
        start = currentOffset * DISPLAY_SIZE;
        rows = DISPLAY_SIZE;
    }

    ui->tableWidget->setRowCount(static_cast<int>(rows));

    // Устанавливаем вертикальные заголовки (номера строк = глобальные индексы)
    for (size_t i = 0; i < rows; ++i) {
        size_t globalIndex = start + i;
        QTableWidgetItem* headerItem = new QTableWidgetItem(QString::number(globalIndex));
        ui->tableWidget->setVerticalHeaderItem(static_cast<int>(i), headerItem);
    }

    for (size_t i = 0; i < rows; ++i) {
        try {
            double val = currentSeq->Get(start + i);
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(val));
            ui->tableWidget->setItem(static_cast<int>(i), 0, item);
        } catch (const IndexOutOfRange&) {
            break;
        } catch (const EndOfSequence&) {
            break;
        } catch (const std::exception& e) {
            qDebug() << "Ошибка чтения" << i << e.what();
            break;
        }
    }
    ui->tableWidget->blockSignals(false);
}

void MainWindow::updateHistogram() {
    if (!currentSeq || !ui->histogramView) {
        if (ui->histogramView) ui->histogramView->setChart(new QChart());
        return;
    }

    Array<double> values;
    if (currentSeq->IsFinite()) {
        for (size_t i = 0; i < currentSeq->GetLength(); ++i)
            values.push_back(currentSeq->Get(i));
    } else {
        size_t start = currentOffset * DISPLAY_SIZE;
        for (size_t i = 0; i < DISPLAY_SIZE; ++i) {
            try {
                values.push_back(currentSeq->Get(start + i));
            } catch (...) { break; }
        }
    }

    if (values.empty()) {
        ui->histogramView->setChart(new QChart());
        return;
    }

    int nbins = std::max(5, std::min(30, static_cast<int>(std::sqrt(values.size()))));
    double minVal = values[0], maxVal = values[0];
    for (size_t i = 1; i < values.size(); ++i) {
        if (values[i] < minVal) minVal = values[i];
        if (values[i] > maxVal) maxVal = values[i];
    }

    if (minVal == maxVal) {
        QChart* chart = new QChart();
        QBarSet* set = new QBarSet("Значения");
        *set << static_cast<double>(values.size());
        QBarSeries* series = new QBarSeries();
        series->append(set);
        chart->addSeries(series);
        chart->setTitle("Гистограмма (все значения равны)");
        chart->createDefaultAxes();
        ui->histogramView->setChart(chart);
        return;
    }

    double binWidth = (maxVal - minVal) / nbins;
    Array<int> counts;
    for (int i = 0; i < nbins; ++i) counts.push_back(0);

    for (size_t i = 0; i < values.size(); ++i) {
        int idx = static_cast<int>((values[i] - minVal) / binWidth);
        if (idx == nbins) idx = nbins - 1;
        if (idx >= 0 && idx < nbins) counts[idx] = counts[idx] + 1;
    }

    QBarSet* barSet = new QBarSet("Частота");
    for (int i = 0; i < nbins; ++i) *barSet << counts[i];

    QBarSeries* series = new QBarSeries();
    series->append(barSet);

    QStringList categories;
    for (int i = 0; i < nbins; ++i) {
        double left = minVal + i * binWidth;
        double right = left + binWidth;
        categories << QString("%1…%2").arg(left, 0, 'g', 2).arg(right, 0, 'g', 2);
    }
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Интервалы");

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Количество");

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    chart->setTitle(QString("Гистограмма распределения (%1 элементов)").arg(values.size()));
    chart->setAnimationOptions(QChart::SeriesAnimations);

    ui->histogramView->setChart(chart);
    ui->histogramView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::logMessage(const QString& msg) {
    ui->listWidget_2->addItem(QDateTime::currentDateTime().toString("hh:mm:ss") + " " + msg);
    ui->listWidget_2->scrollToBottom();
}

void MainWindow::setCurrentSequence(SharedPtr<LazySequence<double>> newSeq, bool addToHistory) {
    if (!newSeq) {
        logMessage("Попытка установить пустую последовательность");
        return;
    }
    currentOffset = 0;
    currentSeq = newSeq;

    // Принудительно материализуем первые DISPLAY_SIZE элементов для бесконечной последовательности
    if (!currentSeq->IsFinite()) {
        for (size_t i = 0; i < DISPLAY_SIZE; ++i) {
            try {
                currentSeq->Get(i);
            } catch (...) { break; }
        }
    }

    updateInfoPanel();
    updateTable();
    enableButtonsBasedOnSequence();
    updateHistogram();

    if (addToHistory) {
        bool found = false;
        for (size_t i = 0; i < history.size(); ++i) {
            if (history[i].get() == newSeq.get()) {
                history[i] = newSeq;
                found = true;
                break;
            }
        }
        if (!found) history.push_back(newSeq);
    }
}

void MainWindow::applyOperation(const QString& opName, SharedPtr<LazySequence<double>> newSeq, bool addToHistory) {
    if (!newSeq) {
        logMessage("Ошибка: " + opName + " не выполнена");
        return;
    }
    setCurrentSequence(newSeq, addToHistory);
    logMessage(opName + " выполнена");
}

void MainWindow::replaceCurrentSequence(SharedPtr<LazySequence<double>> newSeq, const QString& opName, bool preserveOffset) {
    if (!newSeq) {
        logMessage("Ошибка: " + opName + " не выполнена");
        return;
    }

    for (size_t i = 0; i < history.size(); ++i) {
        if (history[i].get() == currentSeq.get()) {
            history[i] = newSeq;
            break;
        }
    }
    if (preserveOffset && !newSeq->IsFinite()) {
        // оставляем currentOffset
    } else {
        currentOffset = 0;
    }
    currentSeq = newSeq;

    // Принудительно материализуем первые DISPLAY_SIZE элементов для бесконечной последовательности
    if (!currentSeq->IsFinite()) {
        for (size_t i = 0; i < DISPLAY_SIZE; ++i) {
            try {
                currentSeq->Get(i);
            } catch (...) { break; }
        }
    }

    updateInfoPanel();
    updateTable();
    enableButtonsBasedOnSequence();
    updateHistogram();
    logMessage(opName + " выполнена");
}