#include "SnailRenderer.h"
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QScreen>
#include <QRandomGenerator>
#include <QCursor>
#include <QMouseEvent>
#include <QPainterPath>
#include <QStyle>
#include <QDateTime>
#include <cmath>
#include <algorithm>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

class Pet : public QWidget {
    Mood mood = Mood::Wander;
    QTimer tick;
    QSystemTrayIcon tray;
    double x = 120, y = 120, vx = 0.65, phase = 0;
    int steps = 0, moodTicks = 0, idleTicks = 0;
    int blinkTicks = 0, nextBlink = 35;
    bool roaming = false;
    double gazeX = 0.0, gazeY = 0.0;
    QRect desktop;
    void moodFor(Mood m, int duration) { mood=m; moodTicks=duration; update(); }
    void resetBounds() { desktop = screen() ? screen()->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry(); }
    void showPet() { show(); raise(); }

public:
    Pet() {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);
        setFixedSize(112,77);
        resetBounds(); x=desktop.left()+80; y=desktop.bottom()-height()+1;
        move(int(x),int(y));
        QMenu *menu=new QMenu(this);
        menu->addAction("🥬 Salatblatt geben",this,[this]{moodFor(Mood::Eat,100);});
        menu->addAction("♥ Streicheln",this,[this]{moodFor(Mood::Pet,60);});
        menu->addAction("⚽ Ball geben",this,[this]{moodFor(Mood::Play,100);});
        menu->addAction("💤 Nickerchen",this,[this]{moodFor(Mood::Sleep,160);});
        menu->addAction("👀 Neugier testen",this,[this]{moodFor(Mood::Look,80);});
        menu->addSeparator();
        menu->addAction("Ausblenden",this,&QWidget::hide);
        menu->addAction("Beenden",qApp,&QApplication::quit);
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this,&QWidget::customContextMenuRequested,this,[this,menu](QPoint pt){menu->popup(mapToGlobal(pt));});
        QMenu *trayMenu=new QMenu(this);
        trayMenu->addAction("Schneggr zeigen",this,[this]{showPet();});
        trayMenu->addAction("Schneggr ausblenden",this,&QWidget::hide);
        trayMenu->addAction("Beenden",qApp,&QApplication::quit);
        QPixmap iconPix(64,64); iconPix.fill(Qt::transparent); {QPainter p(&iconPix);p.setRenderHint(QPainter::Antialiasing);p.setBrush(QColor("#ad79bd"));p.setPen(QPen(QColor("#6d493d"),3));p.drawEllipse(7,7,49,49);}
        tray.setIcon(QIcon(iconPix)); tray.setToolTip("Schneggr"); tray.setContextMenu(trayMenu);
        if(QSystemTrayIcon::isSystemTrayAvailable()) tray.show();
        connect(&tray,&QSystemTrayIcon::activated,this,[this](QSystemTrayIcon::ActivationReason r){if(r==QSystemTrayIcon::Trigger) showPet();});
        connect(&tick,&QTimer::timeout,this,[this]{
            if(!isVisible()) return;
            ++steps; phase+=0.3;
            // Follow nearby pointer movement smoothly; never move the window itself.
            const QPointF delta = QPointF(QCursor::pos()) - QPointF(x + width()/2.0, y + height()/2.0);
            const double targetX = (mood == Mood::Wander && std::abs(delta.x()) < 360 && std::abs(delta.y()) < 240)
                ? std::clamp(delta.x() / 150.0, -1.0, 1.0) : 0.0;
            const double targetY = (mood == Mood::Wander && std::abs(delta.x()) < 240 && std::abs(delta.y()) < 160)
                ? std::clamp(delta.y() / 120.0, -1.0, 1.0) : 0.0;
            gazeX += (targetX - gazeX) * 0.14;
            gazeY += (targetY - gazeY) * 0.14;
            if (blinkTicks > 0) {
                --blinkTicks;
            } else if (--nextBlink <= 0) {
                blinkTicks = 2;
                nextBlink = QRandomGenerator::global()->bounded(30, 81);
            }
            if(moodTicks>0) { --moodTicks; if(moodTicks==0) mood=Mood::Wander; }
            if(mood==Mood::Wander) {
                x+=vx;
                if(x<desktop.left() || x+width()>desktop.right()+1) {vx=-vx; x=std::clamp(x,double(desktop.left()),double(desktop.right()-width()+1));}
                if(steps%250==0) {
                    int r=QRandomGenerator::global()->bounded(10);
                    if(r<2) moodFor(Mood::Eat,100);
                    else if(r<4) moodFor(Mood::Sleep,140);
                    else if(r==4) moodFor(Mood::Look,35);
                    else if(r==5) vx=-vx;
                    else if(r==6) roaming=true;
                }
                if(roaming) {
                    y-=0.45;
                    if(y<desktop.top()+desktop.height()*0.4) roaming=false;
                } else if(y<desktop.bottom()-height()+1) y+=0.45;
                move(int(x),int(y));
            }
            update();
        });
        tick.setInterval(100); tick.start();
    }
protected:
    void paintEvent(QPaintEvent*) override {QPainter p(this); SnailRenderer::draw(p, width(), height(), mood, phase, vx, blinkTicks > 0, gazeX, gazeY);}
    void mousePressEvent(QMouseEvent *e) override {if(e->button()==Qt::LeftButton) moodFor(Mood::Pet,45); QWidget::mousePressEvent(e);}
};
int main(int argc,char**argv) {
    QApplication app(argc,argv); app.setQuitOnLastWindowClosed(false);
    Pet pet; pet.show(); return app.exec();
}
