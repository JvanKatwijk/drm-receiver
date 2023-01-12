/********************************************************************************
** Form generated from reading UI file 'sdrplay-widget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SDRPLAY_2D_WIDGET_H
#define UI_SDRPLAY_2D_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_sdrplayWidget
{
public:
    QFrame *frame;
    QLabel *deviceLabel;
    QLabel *statusLabel;
    QLCDNumber *api_version;
    QCheckBox *agcControl;
    QSpinBox *ppmControl;
    QLabel *serialNumber;
    QComboBox *antennaSelector;
    QSpinBox *lnaGainSetting;
    QLCDNumber *lnaGRdBDisplay;
    QComboBox *tunerSelector;
    QCheckBox *debugControl;
    QLabel *label;
    QLabel *gainsliderLabel;
    QLabel *label_2;
    QSpinBox *GRdBSelector;

    void setupUi(QWidget *sdrplayWidget_v2)
    {
        if (sdrplayWidget_v2->objectName().isEmpty())
            sdrplayWidget_v2->setObjectName(QString::fromUtf8("sdrplayWidget_v2"));
        sdrplayWidget_v2->resize(350, 270);
        frame = new QFrame(sdrplayWidget_v2);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(10, 0, 321, 261));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        deviceLabel = new QLabel(frame);
        deviceLabel->setObjectName(QString::fromUtf8("deviceLabel"));
        deviceLabel->setGeometry(QRect(20, 20, 121, 21));
        statusLabel = new QLabel(frame);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setGeometry(QRect(16, 180, 121, 21));
        api_version = new QLCDNumber(frame);
        api_version->setObjectName(QString::fromUtf8("api_version"));
        api_version->setGeometry(QRect(130, 20, 81, 21));
        api_version->setLineWidth(0);
        api_version->setSegmentStyle(QLCDNumber::Flat);
        agcControl = new QCheckBox(frame);
        agcControl->setObjectName(QString::fromUtf8("agcControl"));
        agcControl->setGeometry(QRect(200, 150, 71, 21));
        QFont font;
        font.setFamily(QString::fromUtf8("DejaVu Sans Mono"));
        font.setBold(true);
        font.setItalic(true);
        font.setWeight(75);
        agcControl->setFont(font);
        ppmControl = new QSpinBox(frame);
        ppmControl->setObjectName(QString::fromUtf8("ppmControl"));
        ppmControl->setGeometry(QRect(10, 80, 91, 31));
        ppmControl->setMinimum(-200);
        ppmControl->setMaximum(200);
        serialNumber = new QLabel(frame);
        serialNumber->setObjectName(QString::fromUtf8("serialNumber"));
        serialNumber->setGeometry(QRect(20, 50, 171, 21));
        antennaSelector = new QComboBox(frame);
        antennaSelector->addItem(QString());
        antennaSelector->addItem(QString());
        antennaSelector->setObjectName(QString::fromUtf8("antennaSelector"));
        antennaSelector->setGeometry(QRect(160, 220, 141, 31));
        antennaSelector->setFont(font);
        lnaGainSetting = new QSpinBox(frame);
        lnaGainSetting->setObjectName(QString::fromUtf8("lnaGainSetting"));
        lnaGainSetting->setGeometry(QRect(10, 150, 101, 31));
        lnaGRdBDisplay = new QLCDNumber(frame);
        lnaGRdBDisplay->setObjectName(QString::fromUtf8("lnaGRdBDisplay"));
        lnaGRdBDisplay->setGeometry(QRect(110, 150, 51, 31));
        lnaGRdBDisplay->setDigitCount(3);
        lnaGRdBDisplay->setSegmentStyle(QLCDNumber::Flat);
        tunerSelector = new QComboBox(frame);
        tunerSelector->addItem(QString());
        tunerSelector->addItem(QString());
        tunerSelector->setObjectName(QString::fromUtf8("tunerSelector"));
        tunerSelector->setGeometry(QRect(10, 220, 141, 31));
        tunerSelector->setFont(font);
        debugControl = new QCheckBox(frame);
        debugControl->setObjectName(QString::fromUtf8("debugControl"));
        debugControl->setGeometry(QRect(200, 180, 88, 21));
        label = new QLabel(frame);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 120, 101, 21));
        label->setFont(font);
        gainsliderLabel = new QLabel(frame);
        gainsliderLabel->setObjectName(QString::fromUtf8("gainsliderLabel"));
        gainsliderLabel->setGeometry(QRect(150, 120, 151, 21));
        gainsliderLabel->setFont(font);
        label_2 = new QLabel(frame);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 190, 171, 21));
        label_2->setFont(font);
        GRdBSelector = new QSpinBox(frame);
        GRdBSelector->setObjectName(QString::fromUtf8("GRdBSelector"));
        GRdBSelector->setGeometry(QRect(180, 80, 106, 35));
        GRdBSelector->setMinimum(20);
        GRdBSelector->setMaximum(59);

        retranslateUi(sdrplayWidget_v2);

        QMetaObject::connectSlotsByName(sdrplayWidget_v2);
    } // setupUi

    void retranslateUi(QWidget *sdrplayWidget_v2)
    {
        sdrplayWidget_v2->setWindowTitle(QCoreApplication::translate("sdrplayWidget", "SDRplay control", nullptr));
        deviceLabel->setText(QCoreApplication::translate("sdrplayWidget", "mirics-SDRplay", nullptr));
        statusLabel->setText(QString());
#if QT_CONFIG(tooltip)
        api_version->setToolTip(QCoreApplication::translate("sdrplayWidget", "<html><head/><body><p>Version of the SDRplay library detected.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        agcControl->setToolTip(QCoreApplication::translate("sdrplayWidget", "<html><head/><body><p>Touch to switch on automatic gain control - based on the setting of the lnastate and the ifgain.</p><p>If switched on, the lnastate switch and the ifgain slider are switched off</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        agcControl->setText(QCoreApplication::translate("sdrplayWidget", "agc", nullptr));
#if QT_CONFIG(tooltip)
        ppmControl->setToolTip(QCoreApplication::translate("sdrplayWidget", "<html><head/><body><p>ppm control. Tells the device the offset (in ppm) of the observed oscillator offset.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        serialNumber->setText(QString());
        antennaSelector->setItemText(0, QCoreApplication::translate("sdrplayWidget", "Antenna A", nullptr));
        antennaSelector->setItemText(1, QCoreApplication::translate("sdrplayWidget", "Antenna B", nullptr));

#if QT_CONFIG(tooltip)
        antennaSelector->setToolTip(QCoreApplication::translate("sdrplayWidget", "<html><head/><body><p>Select an antenna (RSP-II)</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        lnaGainSetting->setToolTip(QCoreApplication::translate("sdrplayWidget", "<html><head/><body><p>Setting the lna state, determining the gain reduction applied in the lna.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        lnaGRdBDisplay->setToolTip(QCoreApplication::translate("sdrplayWidget", "<html><head/><body><p>The display shows the gain reduction in the lna of the device.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        tunerSelector->setItemText(0, QCoreApplication::translate("sdrplayWidget", "Tuner 1", nullptr));
        tunerSelector->setItemText(1, QCoreApplication::translate("sdrplayWidget", "Tuner 2", nullptr));

#if QT_CONFIG(tooltip)
        tunerSelector->setToolTip(QCoreApplication::translate("sdrplayWidget", "<html><head/><body><p>If the device is an rspduo, select the tuner with this combobox</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        debugControl->setText(QCoreApplication::translate("sdrplayWidget", "debug", nullptr));
        label->setText(QCoreApplication::translate("sdrplayWidget", "ppm control", nullptr));
        gainsliderLabel->setText(QCoreApplication::translate("sdrplayWidget", "if gain reduction", nullptr));
        label_2->setText(QCoreApplication::translate("sdrplayWidget", "lna state selector", nullptr));
    } // retranslateUi

};

namespace Ui {
    class sdrplayWidget: public Ui_sdrplayWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SDRPLAY_2D_WIDGET_H
