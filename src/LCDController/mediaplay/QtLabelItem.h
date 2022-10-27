/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTLABELITEM_H_
#define MEDIAPLAY_QTLABELITEM_H_

#include <QtGui/QLabel>
#include <QtCore/QTimeLine>

enum ShowImageEffort{
    NoEffort = 0,
    VShutterEffort,
    HShutterEffort,
	BOXEffort,
	RoundEffort,
	ChessBoxEffort,
	FadeEffort,
	SpreadEffort,

};

class QtLabelItem : public QLabel
{
    Q_OBJECT

public:
    QtLabelItem(QWidget* parent = 0);
    ~QtLabelItem();

    void setImage(const QPixmap& image);
    void setEffect(int val);
    void startDraw();

public slots:
    void	slot_timeOut(int);

protected:
    void	paintEvent ( QPaintEvent * ev );

private:

    QPixmap	mDrawImage;
    QPixmap	mCurrentImage;

    QTimeLine* mTimeline;
    int m_currentHeight;
    int effect;

    static const char *TAG;
};

#endif /* MEDIAPLAY_QTLABELITEM_H_ */
