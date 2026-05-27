#pragma once

#include <QDialog>
#include "YuvFormat.h"

class QLineEdit;
class QComboBox;
class QSpinBox;
class QLabel;

class OpenYuvDialog : public QDialog {
    Q_OBJECT

public:
    explicit OpenYuvDialog(QWidget *parent = nullptr);

    [[nodiscard]] QString filePath() const;
    [[nodiscard]] int imageWidth() const;
    [[nodiscard]] int imageHeight() const;
    [[nodiscard]] YuvFormat yuvFormat() const;

    [[nodiscard]] static bool detectFromFilename(const QString &path,
                                                  int &fmtIndex, int &w, int &h);

private slots:
    void onBrowse();
    void updateExpectedSize();
    void validateInput();

private:
    void tryDetectFromFilename(const QString &path);
    QLineEdit *m_fileEdit = nullptr;
    QComboBox *m_formatCombo = nullptr;
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
    QLabel *m_sizeLabel = nullptr;
    QPushButton *m_okButton = nullptr;
};
