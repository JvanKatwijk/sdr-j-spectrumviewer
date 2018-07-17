/********************************************************************************
** Form generated from reading UI file 'airspy-widget.ui'
**
** Created by: Qt User Interface Compiler version 5.9.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AIRSPY_2D_WIDGET_H
#define UI_AIRSPY_2D_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_airspyWidget
{
public:
    QTabWidget *tabWidget;
    QWidget *tab_1;
    QSlider *sensitivitySlider;
    QLCDNumber *sensitivityDisplay;
    QLCDNumber *sensitivity_lnaDisplay;
    QLCDNumber *sensitivity_mixerDisplay;
    QLCDNumber *sensitivity_vgaDisplay;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QWidget *tab_2;
    QSlider *linearitySlider;
    QLCDNumber *linearityDisplay;
    QLCDNumber *linearity_lnaDisplay;
    QLCDNumber *linearity_mixerDisplay;
    QLCDNumber *linearity_vgaDisplay;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_10;
    QWidget *tab_3;
    QSlider *lnaSlider;
    QLabel *label;
    QLCDNumber *lnaDisplay;
    QSlider *mixerSlider;
    QLCDNumber *mixerDisplay;
    QLabel *label_2;
    QSlider *vgaSlider;
    QLabel *label_3;
    QLCDNumber *vgaDisplay;
    QPushButton *lnaButton;
    QPushButton *mixerButton;
    QPushButton *biasButton;
    QLabel *displaySerial;
    QLabel *label_4;
    QComboBox *rateSelector;

    void setupUi(QWidget *airspyWidget)
    {
        if (airspyWidget->objectName().isEmpty())
            airspyWidget->setObjectName(QStringLiteral("airspyWidget"));
        airspyWidget->resize(423, 302);
        tabWidget = new QTabWidget(airspyWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(10, 90, 401, 191));
        tab_1 = new QWidget();
        tab_1->setObjectName(QStringLiteral("tab_1"));
        sensitivitySlider = new QSlider(tab_1);
        sensitivitySlider->setObjectName(QStringLiteral("sensitivitySlider"));
        sensitivitySlider->setGeometry(QRect(20, 20, 321, 20));
        sensitivitySlider->setMaximum(21);
        sensitivitySlider->setValue(10);
        sensitivitySlider->setOrientation(Qt::Horizontal);
        sensitivityDisplay = new QLCDNumber(tab_1);
        sensitivityDisplay->setObjectName(QStringLiteral("sensitivityDisplay"));
        sensitivityDisplay->setGeometry(QRect(340, 20, 41, 23));
        sensitivityDisplay->setFrameShape(QFrame::NoFrame);
        sensitivityDisplay->setDigitCount(2);
        sensitivityDisplay->setSegmentStyle(QLCDNumber::Flat);
        sensitivity_lnaDisplay = new QLCDNumber(tab_1);
        sensitivity_lnaDisplay->setObjectName(QStringLiteral("sensitivity_lnaDisplay"));
        sensitivity_lnaDisplay->setGeometry(QRect(220, 60, 64, 23));
        sensitivity_lnaDisplay->setFrameShape(QFrame::NoFrame);
        sensitivity_lnaDisplay->setDigitCount(2);
        sensitivity_lnaDisplay->setSegmentStyle(QLCDNumber::Flat);
        sensitivity_mixerDisplay = new QLCDNumber(tab_1);
        sensitivity_mixerDisplay->setObjectName(QStringLiteral("sensitivity_mixerDisplay"));
        sensitivity_mixerDisplay->setGeometry(QRect(220, 80, 64, 23));
        sensitivity_mixerDisplay->setFrameShape(QFrame::NoFrame);
        sensitivity_mixerDisplay->setDigitCount(2);
        sensitivity_mixerDisplay->setSegmentStyle(QLCDNumber::Flat);
        sensitivity_vgaDisplay = new QLCDNumber(tab_1);
        sensitivity_vgaDisplay->setObjectName(QStringLiteral("sensitivity_vgaDisplay"));
        sensitivity_vgaDisplay->setGeometry(QRect(220, 100, 64, 23));
        sensitivity_vgaDisplay->setFrameShape(QFrame::NoFrame);
        sensitivity_vgaDisplay->setDigitCount(2);
        sensitivity_vgaDisplay->setSegmentStyle(QLCDNumber::Flat);
        label_5 = new QLabel(tab_1);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(120, 60, 101, 20));
        label_6 = new QLabel(tab_1);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(120, 80, 91, 20));
        label_7 = new QLabel(tab_1);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(120, 100, 81, 20));
        tabWidget->addTab(tab_1, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        linearitySlider = new QSlider(tab_2);
        linearitySlider->setObjectName(QStringLiteral("linearitySlider"));
        linearitySlider->setGeometry(QRect(20, 20, 301, 20));
        linearitySlider->setMaximum(21);
        linearitySlider->setValue(10);
        linearitySlider->setOrientation(Qt::Horizontal);
        linearityDisplay = new QLCDNumber(tab_2);
        linearityDisplay->setObjectName(QStringLiteral("linearityDisplay"));
        linearityDisplay->setGeometry(QRect(330, 20, 64, 23));
        linearityDisplay->setDigitCount(2);
        linearityDisplay->setSegmentStyle(QLCDNumber::Flat);
        linearity_lnaDisplay = new QLCDNumber(tab_2);
        linearity_lnaDisplay->setObjectName(QStringLiteral("linearity_lnaDisplay"));
        linearity_lnaDisplay->setGeometry(QRect(220, 60, 64, 23));
        linearity_lnaDisplay->setFrameShape(QFrame::NoFrame);
        linearity_lnaDisplay->setDigitCount(2);
        linearity_lnaDisplay->setSegmentStyle(QLCDNumber::Flat);
        linearity_mixerDisplay = new QLCDNumber(tab_2);
        linearity_mixerDisplay->setObjectName(QStringLiteral("linearity_mixerDisplay"));
        linearity_mixerDisplay->setGeometry(QRect(220, 80, 64, 23));
        linearity_mixerDisplay->setFrameShape(QFrame::NoFrame);
        linearity_mixerDisplay->setDigitCount(2);
        linearity_mixerDisplay->setSegmentStyle(QLCDNumber::Flat);
        linearity_vgaDisplay = new QLCDNumber(tab_2);
        linearity_vgaDisplay->setObjectName(QStringLiteral("linearity_vgaDisplay"));
        linearity_vgaDisplay->setGeometry(QRect(220, 100, 64, 23));
        linearity_vgaDisplay->setFrameShape(QFrame::NoFrame);
        linearity_vgaDisplay->setDigitCount(2);
        linearity_vgaDisplay->setSegmentStyle(QLCDNumber::Flat);
        label_8 = new QLabel(tab_2);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(100, 60, 91, 20));
        label_9 = new QLabel(tab_2);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(100, 80, 101, 20));
        label_10 = new QLabel(tab_2);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setGeometry(QRect(100, 100, 71, 20));
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        lnaSlider = new QSlider(tab_3);
        lnaSlider->setObjectName(QStringLiteral("lnaSlider"));
        lnaSlider->setGeometry(QRect(100, 10, 221, 20));
        lnaSlider->setMaximum(15);
        lnaSlider->setValue(10);
        lnaSlider->setOrientation(Qt::Horizontal);
        label = new QLabel(tab_3);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 10, 66, 20));
        lnaDisplay = new QLCDNumber(tab_3);
        lnaDisplay->setObjectName(QStringLiteral("lnaDisplay"));
        lnaDisplay->setGeometry(QRect(343, 10, 51, 23));
        lnaDisplay->setDigitCount(2);
        lnaDisplay->setSegmentStyle(QLCDNumber::Flat);
        mixerSlider = new QSlider(tab_3);
        mixerSlider->setObjectName(QStringLiteral("mixerSlider"));
        mixerSlider->setGeometry(QRect(100, 40, 221, 20));
        mixerSlider->setMaximum(15);
        mixerSlider->setValue(10);
        mixerSlider->setOrientation(Qt::Horizontal);
        mixerDisplay = new QLCDNumber(tab_3);
        mixerDisplay->setObjectName(QStringLiteral("mixerDisplay"));
        mixerDisplay->setGeometry(QRect(343, 40, 51, 23));
        mixerDisplay->setDigitCount(2);
        mixerDisplay->setSegmentStyle(QLCDNumber::Flat);
        label_2 = new QLabel(tab_3);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 40, 66, 20));
        vgaSlider = new QSlider(tab_3);
        vgaSlider->setObjectName(QStringLiteral("vgaSlider"));
        vgaSlider->setGeometry(QRect(100, 70, 221, 20));
        vgaSlider->setMaximum(15);
        vgaSlider->setValue(10);
        vgaSlider->setOrientation(Qt::Horizontal);
        label_3 = new QLabel(tab_3);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 60, 66, 20));
        vgaDisplay = new QLCDNumber(tab_3);
        vgaDisplay->setObjectName(QStringLiteral("vgaDisplay"));
        vgaDisplay->setGeometry(QRect(343, 70, 51, 23));
        vgaDisplay->setDigitCount(2);
        vgaDisplay->setSegmentStyle(QLCDNumber::Flat);
        lnaButton = new QPushButton(tab_3);
        lnaButton->setObjectName(QStringLiteral("lnaButton"));
        lnaButton->setGeometry(QRect(20, 110, 61, 32));
        mixerButton = new QPushButton(tab_3);
        mixerButton->setObjectName(QStringLiteral("mixerButton"));
        mixerButton->setGeometry(QRect(120, 110, 90, 32));
        biasButton = new QPushButton(tab_3);
        biasButton->setObjectName(QStringLiteral("biasButton"));
        biasButton->setGeometry(QRect(250, 110, 90, 32));
        tabWidget->addTab(tab_3, QString());
        displaySerial = new QLabel(airspyWidget);
        displaySerial->setObjectName(QStringLiteral("displaySerial"));
        displaySerial->setGeometry(QRect(100, 60, 241, 21));
        label_4 = new QLabel(airspyWidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(50, 20, 211, 20));
        rateSelector = new QComboBox(airspyWidget);
        rateSelector->setObjectName(QStringLiteral("rateSelector"));
        rateSelector->setGeometry(QRect(270, 10, 141, 31));

        retranslateUi(airspyWidget);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(airspyWidget);
    } // setupUi

    void retranslateUi(QWidget *airspyWidget)
    {
        airspyWidget->setWindowTitle(QApplication::translate("airspyWidget", "airspy", Q_NULLPTR));
        label_5->setText(QApplication::translate("airspyWidget", "lna gain", Q_NULLPTR));
        label_6->setText(QApplication::translate("airspyWidget", "mixer gain", Q_NULLPTR));
        label_7->setText(QApplication::translate("airspyWidget", "vga gain", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_1), QApplication::translate("airspyWidget", "sensitivity", Q_NULLPTR));
        label_8->setText(QApplication::translate("airspyWidget", "lna gain", Q_NULLPTR));
        label_9->setText(QApplication::translate("airspyWidget", "mixer gain", Q_NULLPTR));
        label_10->setText(QApplication::translate("airspyWidget", "vga gain", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("airspyWidget", "linearity", Q_NULLPTR));
        label->setText(QApplication::translate("airspyWidget", "lna", Q_NULLPTR));
        label_2->setText(QApplication::translate("airspyWidget", "mixer", Q_NULLPTR));
        label_3->setText(QApplication::translate("airspyWidget", "vga", Q_NULLPTR));
        lnaButton->setText(QApplication::translate("airspyWidget", "lna", Q_NULLPTR));
        mixerButton->setText(QApplication::translate("airspyWidget", "mixer ", Q_NULLPTR));
        biasButton->setText(QApplication::translate("airspyWidget", "bias", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("airspyWidget", "classic view", Q_NULLPTR));
        displaySerial->setText(QString());
        label_4->setText(QApplication::translate("airspyWidget", "A I R S P Y  handler", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class airspyWidget: public Ui_airspyWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AIRSPY_2D_WIDGET_H
