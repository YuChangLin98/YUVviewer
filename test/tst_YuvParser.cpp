#include <QtTest>
#include <QTemporaryFile>
#include "YuvParser.h"

static QByteArray makeYuv420Data(int w, int h)
{
    int ySize = w * h;
    int uvSize = ySize / 4;
    QByteArray data;
    data.fill('\x80', ySize);
    data.append(QByteArray(uvSize, '\x80'));
    data.append(QByteArray(uvSize, '\x80'));
    return data;
}

class TestYuvParser : public QObject {
    Q_OBJECT

private slots:
    void parseValidYV12()
    {
        int w = 4, h = 4;
        QByteArray data = makeYuv420Data(w, h);

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.write(data);
        tmp.seek(0);

        auto result = YuvParser::parse(tmp, w, h, YuvFormat::YV12);
        QVERIFY2(result.ok(), qPrintable(result.errorMessage));
        QCOMPARE(result.frame.width, w);
        QCOMPARE(result.frame.height, h);
        QCOMPARE(result.frame.yPlane.size(), w * h);
    }

    void parseValidNV12()
    {
        int w = 4, h = 4;
        int ySize = w * h;
        QByteArray data;
        data.fill('\x80', ySize);
        data.append(QByteArray(ySize / 2, '\x80'));

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.write(data);
        tmp.seek(0);

        auto result = YuvParser::parse(tmp, w, h, YuvFormat::NV12);
        QVERIFY(result.ok());
        QCOMPARE(result.frame.width, w);
        QCOMPARE(result.frame.height, h);
    }

    void parseValidNV21()
    {
        int w = 4, h = 4;
        int ySize = w * h;
        QByteArray data;
        data.fill('\x80', ySize);
        data.append(QByteArray(ySize / 2, '\x80'));

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.write(data);
        tmp.seek(0);

        auto result = YuvParser::parse(tmp, w, h, YuvFormat::NV21);
        QVERIFY(result.ok());
    }

    void fileTooSmall()
    {
        int w = 4, h = 4;
        QByteArray data(10, '\x80');

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.write(data);
        tmp.seek(0);

        auto result = YuvParser::parse(tmp, w, h, YuvFormat::YV12);
        QVERIFY(!result.ok());
        QVERIFY(!result.errorMessage.isEmpty());
    }

    void oddDimensions()
    {
        QByteArray data(100, '\x80');

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.write(data);
        tmp.seek(0);

        auto result = YuvParser::parse(tmp, 3, 3, YuvFormat::YV12);
        QVERIFY(!result.ok());
    }

    void negativeDimensions()
    {
        QByteArray data(100, '\x80');

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.write(data);
        tmp.seek(0);

        auto result = YuvParser::parse(tmp, -1, 4, YuvFormat::YV12);
        QVERIFY(!result.ok());
    }
};

QTEST_MAIN(TestYuvParser)
#include "tst_YuvParser.moc"
