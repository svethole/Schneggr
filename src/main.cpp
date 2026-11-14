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
    enum class Mood { Wander, Sleep, Eat, Pet, Play, Look };
    Mood mood = Mood::Wander;
    QTimer tick;
    QSystemTrayIcon tray;
    double x = 120, y = 120, vx = 0.65, phase = 0;
    int steps = 0, moodTicks = 0, idleTicks = 0;
    int blinkTicks = 0, nextBlink = 35;
    bool roaming = false;
    QRect desktop;
    void moodFor(Mood m, int duration) { mood=m; moodTicks=duration; update(); }
    void resetBounds() { desktop = screen() ? screen()->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry(); }
    void showPet() { show(); raise(); }
    void drawPet(QPainter &p) {
        p.setRenderHint(QPainter::Antialiasing);
        p.translate(width()/2.0, height()/2.0 + (mood==Mood::Wander ? std::sin(phase)*1.5 : 0));
        if(vx < 0) p.scale(-1,1);
        p.setPen(QPen(QColor("#6d493d"), 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(QColor("#c5e7a5"));
        QPainterPath body; body.moveTo(-44,25); body.cubicTo(-35,9,-10,8,10,17); body.cubicTo(30,20,37,13,42,10); body.cubicTo(45,25,34,33,10,33); body.lineTo(-38,33); body.closeSubpath(); p.drawPath(body);
        p.setBrush(QColor("#ad79bd")); p.drawEllipse(QRectF(-32,-14,51,49));
        p.setBrush(QColor("#c99dd6")); p.drawEllipse(QRectF(-25,-9,37,37));
        p.setBrush(Qt::NoBrush); p.drawArc(QRectF(-21,-5,29,29), 40*16, 290*16);
        if(mood != Mood::Sleep) {
            p.setPen(QPen(QColor("#6d493d"), 3, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(29,13),QPointF(28,-18)); p.drawLine(QPointF(40,12),QPointF(44,-21));
            if (blinkTicks > 0) {
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
public:
    Pet() {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);
        setFixedSize(160,110);
        resetBounds(); x=desktop.left()+80; y=desktop.bottom()-height()+1;
        move(int(x),int(y));
        QMenu *menu=new QMenu(this);
        menu->addAction("🥬 Salatblatt geben",this,[this]{moodFor(Mood::Eat,100);});
        menu->addAction("♥ Streicheln",this,[this]{moodFor(Mood::Pet,60);});
        menu->addAction("⚽ Ball geben",this,[this]{moodFor(Mood::Play,100);});
        menu->addAction("💤 Nickerchen",this,[this]{moodFor(Mood::Sleep,160);});
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
    void paintEvent(QPaintEvent*) override {QPainter p(this);drawPet(p);}
    void mousePressEvent(QMouseEvent *e) override {if(e->button()==Qt::LeftButton) moodFor(Mood::Pet,45); QWidget::mousePressEvent(e);}
};
int main(int argc,char**argv) {
    QApplication app(argc,argv); app.setQuitOnLastWindowClosed(false);
    Pet pet; pet.show(); return app.exec();
}
