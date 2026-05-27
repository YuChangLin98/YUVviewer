#include "OpenYuvDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QDialogButtonBox>
#include <QRegularExpression>

OpenYuvDialog::OpenYuvDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("打开 YUV 图片"));
    setMinimumWidth(420);

    auto *mainLayout = new QVBoxLayout(this);

    // File selection
    auto *fileGroup = new QGroupBox(QStringLiteral("文件"));
    auto *fileLayout = new QHBoxLayout(fileGroup);

    m_fileEdit = new QLineEdit;
    m_fileEdit->setPlaceholderText(QStringLiteral("选择 YUV 原始文件..."));
    fileLayout->addWidget(m_fileEdit);

    auto *browseBtn = new QPushButton(QStringLiteral("浏览..."));
    connect(browseBtn, &QPushButton::clicked, this, &OpenYuvDialog::onBrowse);
    fileLayout->addWidget(browseBtn);

    mainLayout->addWidget(fileGroup);

    // Image parameters
    auto *paramGroup = new QGroupBox(QStringLiteral("图片参数"));
    auto *paramLayout = new QFormLayout(paramGroup);

    m_formatCombo = new QComboBox;
    m_formatCombo->addItems({"YV12", "NV12", "NV21"});
    paramLayout->addRow(QStringLiteral("格式:"), m_formatCombo);

    m_widthSpin = new QSpinBox;
    m_widthSpin->setRange(2, 16384);
    m_widthSpin->setSingleStep(2);
    m_widthSpin->setValue(1920);
    paramLayout->addRow(QStringLiteral("宽度:"), m_widthSpin);

    m_heightSpin = new QSpinBox;
    m_heightSpin->setRange(2, 16384);
    m_heightSpin->setSingleStep(2);
    m_heightSpin->setValue(1080);
    paramLayout->addRow(QStringLiteral("高度:"), m_heightSpin);

    mainLayout->addWidget(paramGroup);

    // Expected size
    m_sizeLabel = new QLabel;
    mainLayout->addWidget(m_sizeLabel);

    // Dialog buttons
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setText(QStringLiteral("打开"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Signal connections for validation
    connect(m_fileEdit, &QLineEdit::textChanged, this, &OpenYuvDialog::validateInput);
    connect(m_widthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OpenYuvDialog::validateInput);
    connect(m_heightSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OpenYuvDialog::validateInput);

    connect(m_widthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OpenYuvDialog::updateExpectedSize);
    connect(m_heightSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OpenYuvDialog::updateExpectedSize);

    updateExpectedSize();
    validateInput();
}

void OpenYuvDialog::onBrowse()
{
    QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("打开 YUV 文件"), QString(),
        QStringLiteral("YUV 文件 (*.yuv *.nv12 *.nv21);;所有文件 (*)"));

    if (!path.isEmpty()) {
        m_fileEdit->setText(path);
        tryDetectFromFilename(path);
    }
}

bool OpenYuvDialog::detectFromFilename(const QString &path,
                                        int &fmtIndex, int &w, int &h)
{
    QString name = QFileInfo(path).fileName().toLower();

    fmtIndex = 1; // default NV12
    if (name.contains("yv12") || name.contains("yuv420p"))
        fmtIndex = 0;
    else if (name.contains("nv12"))
        fmtIndex = 1;
    else if (name.contains("nv21"))
        fmtIndex = 2;

    static const QRegularExpression re(R"((\d{2,5})[x_](\d{2,5}))");
    auto match = re.match(name);
    if (match.hasMatch()) {
        int mw = match.captured(1).toInt();
        int mh = match.captured(2).toInt();
        if (mw >= 2 && mh >= 2 && mw <= 16384 && mh <= 16384) {
            w = mw;
            h = mh;
            return true;
        }
    }
    return false;
}

void OpenYuvDialog::tryDetectFromFilename(const QString &path)
{
    int fmtIndex, w = 0, h = 0;
    bool dimsDetected = detectFromFilename(path, fmtIndex, w, h);
    m_formatCombo->setCurrentIndex(fmtIndex);
    if (dimsDetected) {
        m_widthSpin->setValue(w);
        m_heightSpin->setValue(h);
    }
}

void OpenYuvDialog::updateExpectedSize()
{
    int w = m_widthSpin->value();
    int h = m_heightSpin->value();
    qint64 bytes = static_cast<qint64>(w) * h * 3 / 2;
    double mb = bytes / (1024.0 * 1024.0);
    m_sizeLabel->setText(QStringLiteral("预期文件大小: %1 MB (%2 字节)")
                             .arg(mb, 0, 'f', 2).arg(bytes));
}

void OpenYuvDialog::validateInput()
{
    bool fileExists = QFileInfo::exists(m_fileEdit->text());
    bool validDims = m_widthSpin->value() >= 2 && m_heightSpin->value() >= 2;
    m_okButton->setEnabled(fileExists && validDims);
}

QString OpenYuvDialog::filePath() const { return m_fileEdit->text(); }
int OpenYuvDialog::imageWidth() const { return m_widthSpin->value(); }
int OpenYuvDialog::imageHeight() const { return m_heightSpin->value(); }

YuvFormat OpenYuvDialog::yuvFormat() const
{
    return static_cast<YuvFormat>(m_formatCombo->currentIndex());
}
