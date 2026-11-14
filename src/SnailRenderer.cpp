#include "SnailRenderer.h"
#include <QPainterPath>
#include <cmath>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>

void SnailRenderer::draw(QPainter &p, int width, int height, Mood mood, double phase, double vx, bool blinking, double gazeX, double gazeY, bool prototypeSprites) {
        p.setRenderHint(QPainter::Antialiasing);
        p.translate(width/2.0, height/2.0 + (mood==Mood::Wander ? std::sin(phase)*1.5 : 0));
        p.scale(0.7, 0.7);
        if(vx < 0) p.scale(-1,1);
        // Optional CGI artwork: keep the vector snail as a fallback until the asset is installed.
        const auto loadArtwork = [](const QString &filename) -> QPixmap {
            const QStringList paths = {
                QDir::current().filePath(QStringLiteral("assets/") + filename),
                QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("assets/") + filename),
                QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../assets/") + filename)
            };
            for (const QString &path : paths) {
                if (QFileInfo(path).isFile()) {
                    QPixmap pixmap(path);
                    if (!pixmap.isNull()) return pixmap;
                }
            }
            return {};
        };
        static const QPixmap cgi = loadArtwork(QStringLiteral("schneggr.png"));
        static const QPixmap eyelids = loadArtwork(QStringLiteral("schneggr_eyelids.png"));
        static const QStringList spriteNames = {
            QStringLiteral("crawl_00"), QStringLiteral("crawl_01"),
            QStringLiteral("crawl_02"), QStringLiteral("crawl_03"),
            QStringLiteral("gaze_left"), QStringLiteral("gaze_center"),
            QStringLiteral("gaze_right"), QStringLiteral("gaze_up"),
            QStringLiteral("face_smile"), QStringLiteral("face_surprised"),
            QStringLiteral("face_bored"), QStringLiteral("face_sleep")
        };
        static const QVector<QPixmap> sprites = [&loadArtwork] {
            QVector<QPixmap> frames;
            for (const QString &name : spriteNames)
                frames.append(loadArtwork(name + QStringLiteral(".png")));
            return frames;
        }();
        if (!cgi.isNull()) {
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            // Subtle whole-image movement; the CGI asset is still a single static frame.
            // Keep changes small so the snail stays within its transparent window.
            p.save();
            if (mood == Mood::Wander) {
                const double wave = std::sin(phase);
                p.scale(1.0 + 0.012 * wave, 1.0 - 0.018 * wave);
                p.rotate(0.8 * std::sin(phase * 0.5));
            } else if (mood == Mood::Sleep) {
                // Slow, nearly imperceptible breathing, with eyelids held shut.
                const double breath = std::sin(phase * 0.12);
                p.translate(0, -2.0 * breath);
                p.scale(1.0 + 0.008 * breath, 1.0 - 0.045 * breath);
            } else if (mood == Mood::Pet || mood == Mood::Play) {
                p.translate(0, -1.5 * std::abs(std::sin(phase)));
            } else if (mood == Mood::Look) {
                // Keep the body still until the eyestalks have their own sprite layer.
            } else if (mood == Mood::Eat) {
                p.rotate(0.6 * std::sin(phase * 1.5));
            }
            // Preserve the artwork's aspect ratio inside the existing transparent pet window.
            const QRectF bounds(-73, -40, 146, 82);
            // Until a pixel-aligned eyelid overlay is available, keep the original
            // CGI image unchanged to avoid flashes in color and lighting.
            const QPixmap *art = &cgi;
            if (prototypeSprites) {
                int frame = 8;
                if (mood == Mood::Wander) {
                    frame = (std::abs(gazeX) > 0.35 || gazeY < -0.5)
                        ? (gazeY < -0.5 ? 7 : (gazeX < 0 ? 4 : 6))
                        : (static_cast<int>(std::floor(phase * 1.6)) % 4);
                } else if (mood == Mood::Look) frame = 5;
                else if (mood == Mood::Sleep) frame = 11;
                else if (mood == Mood::Eat) frame = static_cast<int>(phase * 1.8) % 2 ? 8 : 9;
                else if (mood == Mood::Play) frame = 9;
                else if (mood == Mood::Pet) frame = 8;
                if (frame >= 0 && frame < sprites.size() && !sprites[frame].isNull())
                    art = &sprites[frame];
            }
            p.drawPixmap(bounds, *art, QRectF(art->rect()));
            if (art == &cgi && (blinking || mood == Mood::Sleep) && !eyelids.isNull()) {
                // The transparent eyelid layer uses the original image coordinates.
                p.drawPixmap(bounds, eyelids, QRectF(eyelids.rect()));
            }
            p.restore();

            // Simple mood indicators until dedicated CGI animation frames exist.
            if (mood == Mood::Sleep || mood == Mood::Pet) {
                p.save();
                QFont font = p.font();
                font.setBold(true);
                font.setPixelSize(mood == Mood::Sleep ? 17 : 20);
                p.setFont(font);
                p.setPen(mood == Mood::Sleep ? QColor("#7764a5") : QColor("#dc5c83"));
                p.drawText(QPointF(-50, -25), mood == Mood::Sleep
                           ? QStringLiteral("Zz") : QStringLiteral("♥"));
                p.restore();
            }
            if (mood == Mood::Play) {
                p.save();
                p.setPen(QPen(QColor("#96563f"), 1.5));
                p.setBrush(QColor("#f5a96e"));
                p.drawEllipse(QRectF(49, 24, 12, 12));
                p.restore();
            }
            return;
        }
        p.setPen(QPen(QColor("#6d493d"), 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(QColor("#c5e7a5"));
        QPainterPath body; body.moveTo(-44,25); body.cubicTo(-35,9,-10,8,10,17); body.cubicTo(30,20,37,13,42,10); body.cubicTo(45,25,34,33,10,33); body.lineTo(-38,33); body.closeSubpath(); p.drawPath(body);
        p.setBrush(QColor("#ad79bd")); p.drawEllipse(QRectF(-32,-14,51,49));
        p.setBrush(QColor("#c99dd6")); p.drawEllipse(QRectF(-25,-9,37,37));
        p.setBrush(Qt::NoBrush); p.drawArc(QRectF(-21,-5,29,29), 40*16, 290*16);
        if(mood != Mood::Sleep) {
            p.setPen(QPen(QColor("#6d493d"), 3, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(29,13),QPointF(28,-18)); p.drawLine(QPointF(40,12),QPointF(44,-21));
            if (blinking) {
                p.setPen(QPen(QColor("#6d493d"), 2.5, Qt::SolidLine, Qt::RoundCap));
                p.drawArc(QRectF(22,-19,15,8), 180*16, 180*16);
                p.drawArc(QRectF(38,-23,15,8), 180*16, 180*16);
            } else {
                p.setBrush(QColor("#f6f3dc")); p.setPen(QPen(QColor("#6d493d"),2));
                p.drawEllipse(QRectF(20,-29,17,17)); p.drawEllipse(QRectF(36,-33,17,17));
                p.setBrush(QColor("#342d32")); p.setPen(Qt::NoPen);
                p.drawEllipse(QRectF(29,-24,5,7)); p.drawEllipse(QRectF(45,-28,5,7));
            }
        } else {
            p.setPen(QPen(QColor("#6d493d"), 2, Qt::SolidLine, Qt::RoundCap));
            p.drawArc(QRectF(22,4,13,7),0,180*16); p.drawArc(QRectF(36,3,13,7),0,180*16);
        }
        p.setPen(QPen(QColor("#6d493d"),2)); p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(33,10,12,9),190*16,150*16);
        if(mood==Mood::Eat) {
            p.setPen(QPen(QColor("#54894e"),1.5)); p.setBrush(QColor("#77c36d"));
            QPainterPath leaf; leaf.moveTo(45,25); leaf.cubicTo(55,5,70,12,69,28); leaf.cubicTo(60,37,51,34,45,25); p.drawPath(leaf);
        }
        if(mood==Mood::Sleep) {
            p.setPen(QColor("#685c9a")); QFont f=p.font(); f.setBold(true); f.setPointSize(15); p.setFont(f); p.drawText(QPointF(17,-34),"Zz");
        }
        if(mood==Mood::Play) { p.setBrush(QColor("#f5a96e")); p.setPen(QPen(QColor("#96563f"),2)); p.drawEllipse(QRectF(54,14,18,18)); }
        if(mood==Mood::Pet) { p.setPen(QColor("#db668b")); QFont f=p.font(); f.setPointSize(17); p.setFont(f); p.drawText(QPointF(5,-37),"♥"); }
    }
