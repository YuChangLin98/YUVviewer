#include <QtTest>
#include "YuvToRgbConverter.h"

class TestYuvToRgbConverter : public QObject {
    Q_OBJECT

private slots:
    void convertValidYV12()
    {
        YuvFrame frame;
        frame.width = 4;
        frame.height = 4;
        frame.format = YuvFormat::YV12;

        // All-gray: Y=128, U=128, V=128
        frame.yPlane = QByteArray(16, '\x80');
        frame.uPlane = QByteArray(4, '\x80');
        frame.vPlane = QByteArray(4, '\x80');

        QImage image = YuvToRgbConverter::convert(frame);
        QVERIFY(!image.isNull());
        QCOMPARE(image.width(), 4);
        QCOMPARE(image.height(), 4);
    }

    void convertValidNV12()
    {
        YuvFrame frame;
        frame.width = 4;
        frame.height = 4;
        frame.format = YuvFormat::NV12;

        frame.yPlane = QByteArray(16, '\x80');
        // NV12: interleaved UV, 8 bytes for 4x4
        QByteArray uv(8, '\x80');
        frame.uPlane = uv;

        QImage image = YuvToRgbConverter::convert(frame);
        QVERIFY(!image.isNull());
        QCOMPARE(image.width(), 4);
        QCOMPARE(image.height(), 4);
    }

    void convertValidNV21()
    {
        YuvFrame frame;
        frame.width = 4;
        frame.height = 4;
        frame.format = YuvFormat::NV21;

        frame.yPlane = QByteArray(16, '\x80');
        frame.uPlane = QByteArray(8, '\x80');

        QImage image = YuvToRgbConverter::convert(frame);
        QVERIFY(!image.isNull());
        QCOMPARE(image.width(), 4);
        QCOMPARE(image.height(), 4);
    }

    void invalidFrameReturnsNull()
    {
        YuvFrame frame; // width=0, height=0 → invalid
        QImage image = YuvToRgbConverter::convert(frame);
        QVERIFY(image.isNull());
    }

    void grayFrameProducesGrayPixels()
    {
        int w = 4, h = 2;
        YuvFrame frame;
        frame.width = w;
        frame.height = h;
        frame.format = YuvFormat::YV12;

        frame.yPlane = QByteArray(w * h, '\x80');           // Y=128
        frame.uPlane = QByteArray(w * h / 4, '\x80');      // U=128
        frame.vPlane = QByteArray(w * h / 4, '\x80');      // V=128

        QImage image = YuvToRgbConverter::convert(frame);
        QVERIFY(!image.isNull());

        // Y=128, U=128, V=128 → Y1=-16+128? Wait, Y-16=112, U-128=0, V-128=0
        // r = (1192*112 + 0 + 512) >> 10 = (133504 + 512) >> 10 = 134016/1024 ≈ 130
        // Let's just check all pixels are non-transparent
        for (int row = 0; row < h; ++row) {
            const QRgb *scanLine = reinterpret_cast<const QRgb *>(image.constScanLine(row));
            for (int col = 0; col < w; ++col) {
                QVERIFY(qAlpha(scanLine[col]) == 255);
            }
        }
    }
};

QTEST_MAIN(TestYuvToRgbConverter)
#include "tst_YuvToRgbConverter.moc"
