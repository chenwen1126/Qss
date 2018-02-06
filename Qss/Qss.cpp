/**
*	@file:	QssTtitleBar.h 
*	@details:	css控件定制源文件，包括所有UI控件及自定义标题栏的主窗口，对话框及消息框
*	@author: 	chenwen(chenwen1126@tom.com)
*   @datetime:  2017-7-25
*   @history:   v1.0 first edition(Qt4.7)
*/
#include "Qss.h"

#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QtMath>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QDebug>
#include <QFile>

#define QSSDIALOG_SHADOW_WIDTH 12 //注意与QFrame#dialog,QFrame#messagebox 的padding属性值相等
#define QSSDIALOG_BODER_WIDTH 0

QssTtitleBar::QssTtitleBar(QWidget *parent , QTitleBar_Type type/* = QTitleBar_Type_Window*/)
	: QWidget(parent),m_maxOrRestore(false),m_pressed(false),m_type(type)
{
	setObjectName("titlebar");

	m_closeBtn = new QPushButton(this);//关闭按钮
	m_closeBtn->setObjectName("titlebarclosebtn");//设置控件名称，方便css指定样式
	m_closeBtn->setToolTip(QString::fromLocal8Bit("关闭"));
	m_closeBtn->setVisible(m_type & QTitleBar_Button_Close);

	m_minBtn = new QPushButton(this);//最小化按钮
	m_minBtn->setObjectName("titlebarminbtn");//设置控件名称，方便css指定样式
	m_minBtn->setToolTip(QString::fromLocal8Bit("最小化"));
	m_minBtn->setVisible(m_type & QTitleBar_Button_Min);

	m_restoreBtn = new QPushButton(this);//最大化按钮，在最大化时隐藏
	m_restoreBtn->setObjectName("titlebarrestorebtn");//设置控件名称，方便css指定样式
	m_restoreBtn->setToolTip(QString::fromLocal8Bit("还原"));
	m_restoreBtn->setVisible(m_type & QTitleBar_Button_Restore);

	m_maxBtn = new QPushButton(this);//恢复按钮，在normal时隐藏
	m_maxBtn->setObjectName("titlebarmaxbtn");//设置控件名称，方便css指定样式
	m_maxBtn->setToolTip(QString::fromLocal8Bit("最大化"));
	m_maxBtn->setVisible(m_type & QTitleBar_Button_Max);

	m_iconBtn = new QPushButton(this);//标题栏图标
	m_iconBtn->setObjectName("titlebaricon");//设置控件名称，方便css指定样式

	m_titlebarTitle = new QLabel(this);//标题栏标题
	m_titlebarTitle->setObjectName("titlebartitle");//设置控件名称，方便css指定样式

	/** 将上面的元素加入一个水平布局 */
	QHBoxLayout* hBox = new QHBoxLayout(this);
	hBox->setMargin(0);
	hBox->addWidget(m_iconBtn);
	hBox->addWidget(m_titlebarTitle);
	hBox->addStretch(1);//水平占位
	hBox->addWidget(m_minBtn);
	hBox->addWidget(m_restoreBtn);
	m_restoreBtn->setVisible(m_maxOrRestore);//默认隐藏
	hBox->addWidget(m_maxBtn);
	hBox->addWidget(m_closeBtn);

	hBox->setSpacing(0);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);//设置标题栏尺寸策略，水平为扩展，垂直为固定

	connect(m_closeBtn, SIGNAL(clicked()), parent, SLOT(close()));//关闭按钮单击时，父窗口关闭
	connect(m_minBtn, SIGNAL(clicked()), parent, SLOT(showMinimized()));
	connect(m_maxBtn, SIGNAL(clicked()), this, SLOT(onMaxOrRestore()));//最大化和正常显示
	connect(m_restoreBtn, SIGNAL(clicked()), this, SLOT(onMaxOrRestore()));

	m_iconBtn->installEventFilter(this);//安装事件过滤，截取m_iconLab上的双击时间，关闭窗口
	installEventFilter(this);//安装事件过滤，截取标题栏上的双击事件，最大化和正常显示显示窗口

	m_rcValid = QApplication::desktop()->availableGeometry(); 
}

QssTtitleBar::~QssTtitleBar()
{
	//qDebug("=========QTitleBar-0x%x:deleteed!!========", this);
}

void QssTtitleBar::setTitle( QString title )
{
	m_titlebarTitle->setText(title);
}

void QssTtitleBar::setIcon( QIcon icon)
{
	m_iconBtn->setIcon(icon);
}

void QssTtitleBar::setMaxOrRestore( bool val)
{
	m_maxOrRestore = val;//true已最大化

	if ((m_type & QTitleBar_Button_Restore) && (m_type & QTitleBar_Button_Max))
	{
		m_restoreBtn->setVisible(m_maxOrRestore);
		m_maxBtn->setVisible(!m_maxOrRestore);
	}
}

/** m_maxNormal为true表示窗口已最大化显示 */
void QssTtitleBar::onMaxOrRestore()
{
	if (m_type != QTitleBar_Type_MainWindow)
		return ;
	
	if (m_maxOrRestore)
	{
		if (!m_rcNormal.isValid())
		{
			QSize sizeHint = parentWidget()->sizeHint();
			if (sizeHint.width() > m_rcValid.width())
				sizeHint.setWidth(m_rcValid.width() -100);
			if (sizeHint.height() > m_rcValid.height())
				sizeHint.setHeight(m_rcValid.height() - 100);

			m_rcNormal.setLeft((m_rcValid.width() - sizeHint.width())/2);
			m_rcNormal.setTop((m_rcValid.height() - sizeHint.height())/2);
			m_rcNormal.setWidth(sizeHint.width());
			m_rcNormal.setHeight(sizeHint.height());
		}
		parentWidget()->setGeometry(m_rcNormal);
	}
	else 
	{
		/** parentWidget()即QWindowFrame去掉标题栏后的最大化显示时成了全屏显示
		这里只好重新设置了最大化显示时的显示尺寸未桌面有效区，但这也带来了窗口
		显示状态的不准确性，导致窗口系统菜单（包括窗口任务栏系统菜单）不能正常
		工作（关闭菜单项除外），状态栏调整大小的三角标不能正确现实的弊端，暂时没有解决办法！！*/
		QRect rc = parentWidget()->geometry();
		if (rc.width() < m_rcValid.width() && rc.height() < m_rcValid.height())
  			m_rcNormal = rc;
		parentWidget()->setGeometry(m_rcValid);
	}

	setMaxOrRestore(!m_maxOrRestore);
}

/** 需要通过QWidget#来设置标题栏css样式，而要设置QWidget CSS样式必须有以下固定形式的代码。
下面的代码在没有指定QWidget CSS样式时上面也不做 */
void QssTtitleBar::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

/** mouseMoveEvent mousePressEvent两个函数都是为了实现在标题栏拖动窗口 
注意mouseMoveEvent事件在未设置setMouseTracking时，只有按住鼠标键时才会发生*/
void QssTtitleBar::mouseMoveEvent( QMouseEvent * ev )
{
	if (m_maxOrRestore)
		return;
	if (!m_pressed)
		return;

	QPoint globalPt = ev->globalPos();
	QPoint movePt = globalPt - m_pressedPos;//整个Frame窗口左上角的globalPos
	parentWidget()->move(movePt);//移动到指定globalPos

	return ;
}

void QssTtitleBar::mousePressEvent( QMouseEvent * ev )
{
	m_pressed = true;
	m_pressedPos = mapToParent(ev->pos());

	return ;
}

void QssTtitleBar::mouseReleaseEvent( QMouseEvent * ev )
{
	m_pressed = false;
}

/** 中途截取标题栏双击消息和标题栏图标双击消息，分别用于处理最大最小化切换和双击标题栏图标关闭窗口 */
bool QssTtitleBar::eventFilter( QObject * obj, QEvent * ev )
{
	if (obj == this)
	{
		if (ev->type() == QEvent::MouseButtonDblClick)
		{
			onMaxOrRestore();
			return true;
		}
	}
	else if (obj == m_iconBtn)
	{
		if (ev->type() == QEvent::MouseButtonDblClick)
		{
			parentWidget()->close();
			return true;
		}
	}

	return QWidget::eventFilter(obj, ev);
}

/** 自定标题栏和边框的窗口的策略是，在一个QWindowFrame中部署垂直布局再分别加入自定标题栏和窗口内容 */
QssMainWindow::QssMainWindow(QWidget *parent/* = 0*/, Qt::WindowFlags flags/* = 0*/)
: QMainWindow(parent, flags),m_mousePressedInBoundy(false)
{
	m_rcValid = QApplication::desktop()->availableGeometry(); 
	
	m_frame = new QFrame(parent, flags);
	//设置控件名称，方便css指定样式
	m_frame->setObjectName("window");
	/** frame设置为无框架（QFrame无父控件时有框架），隐藏标题栏和边框，以便自定义标题栏和边框 
	Qt::WindowSystemMenuHint 指定frame带系统菜单，只不过系统菜单中只有最小化和关闭可用*/
	//WindowMinimizeButtonHint使得系统菜单中最小化和还原菜单项可用，但是用还原菜单项目时还原窗口尺寸不对，
	//因为还有窗口尺寸时我们自定义的m_rcNormal。系统并不知道，而是直接还原成最大尺寸。
	m_frame->setWindowFlags(Qt::Window | Qt::FramelessWindowHint| Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
	//鼠标经过控件时有mouseMoveEvent,否则只有鼠标按键同时才有mouseMoveEvent事件，MouseTracking默认为false
	//这里开MouseTracking主要为了鼠标经过边框时，鼠标指针变为可调整大小的鼠标指针样式
	m_frame->setMouseTracking(true);
	m_frame->installEventFilter(this);
	
	m_titleBar = new QssTtitleBar(m_frame);
	m_titleBar->installEventFilter(this);

	/** QFrame的垂直布局中加入标题栏和去掉了标题栏和边框的窗口内容，这样QFrame就成了我们看到的自定了标题栏和边框的窗口 */
	QVBoxLayout* vbox = new QVBoxLayout(m_frame);
	vbox->setMargin(0);
	vbox->setSpacing(0);
	vbox->addWidget(m_titleBar);
	vbox->addWidget(this);

 	installEventFilter(this);

	/** 动画效果 */

}

QssMainWindow::~QssMainWindow()
{
	m_frame->deleteLater();
}

void QssMainWindow::show()
{
	m_frame->show();
}

void QssMainWindow::showMinimized()
{
	m_frame->showMinimized();
}

void QssMainWindow::showMaximized()
{
	m_titleBar->setMaxOrRestore(true);
	m_frame->setGeometry(m_rcValid);
}

void QssMainWindow::showFullScreen()
{
	m_titleBar->setMaxOrRestore(true);
	m_frame->showFullScreen();
}

void QssMainWindow::showNormal()
{
	m_titleBar->setMaxOrRestore(false);
	m_frame->resize(rect().width(), rect().height() + m_titleBar->rect().height());
	m_frame->showNormal();
}

void QssMainWindow::setWindowTitle( QString title )
{
	m_frame->setWindowTitle(title);
	m_titleBar->setTitle(title);
}

void QssMainWindow::setWindowIcon( QIcon icon )
{
	m_frame->setWindowIcon(icon);
	m_titleBar->setIcon(icon);
}

void QssMainWindow::onMouseMoveEvent( QMouseEvent * ev )
{
	if (m_titleBar->maxOrRestore())
	{
		return;
	}

	if (m_mousePressedInBoundy)
	{
		int x = ev->globalPos().x();
		int y = ev->globalPos().y();

		int dx = x - m_pos.x();
		int dy = y - m_pos.y();

		if ((m_left || m_right) && qAbs(dx) < 5)
			return;
		if ((m_top || m_bottom) && qAbs(dy) < 5)
			return;
		if (m_left && dx > 0 && m_rect.width() <= m_frame->minimumWidth())
			return ;
		if (m_top && dy > 0 && m_rect.height() <= m_frame->minimumHeight())
			return;

		QRect rc = m_rect;
		if (m_left)
			rc.setLeft(rc.left() + dx);
		if (m_right)
			rc.setRight(rc.right() + dx);
		if (m_top)
			rc.setTop(rc.top() + dy);
		if (m_bottom)
			rc.setBottom(rc.bottom() + dy);

		m_frame->setGeometry(rc);
		m_rect = rc;
		m_pos = ev->globalPos();
	}
	else
	{
		int x = ev->x();
		int y = ev->y();

		QRect rc = m_frame->rect();
		m_left = qAbs(x - rc.left()) <= 5;
		m_right = qAbs(x - rc.right()) <= 5;
		m_top = qAbs(y - rc.top()) <= 5;
		m_bottom = qAbs(y - rc.bottom()) <= 5;

		if ((m_left && m_top) || (m_right && m_bottom))
			m_frame->setCursor(Qt::SizeFDiagCursor);
		else if ((m_right && m_top) || (m_left && m_bottom))
			m_frame->setCursor(Qt::SizeBDiagCursor);
		else if ((m_left && !m_top && !m_bottom) || (m_right && !m_top && !m_bottom))
			m_frame->setCursor(Qt::SizeHorCursor);
		else if ((m_top && !m_right && !m_left) || (m_bottom && !m_right && !m_left))
			m_frame->setCursor(Qt::SizeVerCursor);
		else
			m_frame->setCursor(Qt::ArrowCursor);
	}
}

void QssMainWindow::onMousePressEvent( QMouseEvent * ev )
{
	m_pos = ev->globalPos();
	m_rect = m_frame->geometry();
	m_mousePressedInBoundy = (ev->button() == Qt::LeftButton) && (m_left || m_right || m_top || m_bottom);
}

void QssMainWindow::onMouseReleaseEvent( QMouseEvent * ev )
{
	m_mousePressedInBoundy = false;
}

bool QssMainWindow::eventFilter( QObject * obj, QEvent * ev )
{
	if (obj == m_frame)
	{
		if (ev->type() == QEvent::MouseMove)
		{
			QMouseEvent * mouseEv = dynamic_cast<QMouseEvent *>(ev);
			if (ev)
			{
				onMouseMoveEvent(mouseEv);
				return true;
			}
		}
		else if (ev->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent * mouseEv = dynamic_cast<QMouseEvent *>(ev);
			if (ev)
			{
				onMousePressEvent(mouseEv);
				return true;
			}
		}
		else if (ev->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent * mouseEv = dynamic_cast<QMouseEvent *>(ev);
			if (ev)
			{
				onMouseReleaseEvent(mouseEv);
				return true;
			}
		}
		else if (ev->type() == QEvent::Show)
		{
			/* 显示时屏幕居中显示 */
			QRect rc = m_frame->rect();
			QRect parentRc = m_rcValid;

 			/** m_frame总是无父控件的，所以要相对屏幕坐标移动，坐标不能为负 */
 			int x = parentRc.left() + (parentRc.width() - rc.width())*0.5; x = x <= 0 ? 1 : x;
 			int y = parentRc.top() + (parentRc.height() - rc.height())*0.5; y = y <= 0 ? 1 : y;
 			m_frame->move(x, y);

			//对QssMainWindow不能使用显示属性动画，因为使用属性动画会导致QssMainWindow非正常地显示为最小尺寸
		}
 		else if (ev->type() == QEvent::Close)
 		{
 			close();
 			m_titleBar->close();

			return true;
 		}
	}
	else if (obj == m_titleBar)
	{
		if (ev->type() == QEvent::Enter)
		{
			m_left = false;m_right = false; m_top = false; m_bottom = false;
			if (m_frame->cursor().shape() != Qt::ArrowCursor)
				m_frame->setCursor(Qt::ArrowCursor);
		}
	}
	else if (obj == this)
	{
		if (ev->type() == QEvent::Enter)
		{
			m_left = false;m_right = false; m_top = false; m_bottom = false;
			if (m_frame->cursor().shape() != Qt::ArrowCursor)
				m_frame->setCursor(Qt::ArrowCursor);
		}
	}

	return QMainWindow::eventFilter(obj, ev);
}

QssDialog::QssDialog(QWidget *parent)
	: QDialog(0),
	m_mousePressedInBorder(false),
	m_parent(parent)
{
	m_rcValid = QApplication::desktop()->availableGeometry(); 

	m_frame = new QFrame(parent);
	m_frame->setObjectName("dialog");//设置控件名称，方便css指定样式
	m_frame->setAttribute(Qt::WA_TranslucentBackground);/** 设置内容区外透明背景，padding出阴影边框绘制控件 。注意设置该属性后css boder将失效，绘制阴影时同时绘制边框*/
	m_frame->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint| Qt::WindowSystemMenuHint/* | Qt::WindowMinimizeButtonHint*/);//加上Qt::WindowMinimizeButtonHint属性后，dialog通过任务栏最小化和还原后，调整边框处理问题，所以这里暂时不加上该属性
	m_frame->setMouseTracking(true);
	m_frame->installEventFilter(this);

	m_titleBar = new QssTtitleBar(m_frame, QssTtitleBar::QTitleBar_Type_Dialog);
	m_titleBar->installEventFilter(this);

	QVBoxLayout* vbox = new QVBoxLayout(m_frame);
	vbox->setMargin(0);
	vbox->setSpacing(0);
	vbox->addWidget(m_titleBar);
	vbox->addWidget(this);

	installEventFilter(this);
}

QssDialog::~QssDialog()
{
	m_frame->deleteLater();
}

void QssDialog::show()
{
	/** 根据标题栏和内容真实尺寸resize m_frame，否则m_frame将显示sizehint尺寸 */
	int offset = (QSSDIALOG_SHADOW_WIDTH + QSSDIALOG_BODER_WIDTH)*2;//rect()获得尺寸仅为内容区尺寸，不包括边框和padding尺寸。所以要加上边框和padding尺寸偏移才能获取正确的frame尺寸
	m_frame->resize(rect().width() + offset, rect().height() + m_titleBar->rect().height() + offset);

	QDialog::show();
	m_frame->show();
}

void QssDialog::raise()
{
	m_frame->raise();
}

void QssDialog::activateWindow()
{
	m_frame->activateWindow();
}

int QssDialog::exec()
{
	/** 根据标题栏和内容真实尺寸resize m_frame，否则m_frame将显示sizehint尺寸 */
	int offset = (QSSDIALOG_SHADOW_WIDTH + QSSDIALOG_BODER_WIDTH)*2;//rect()获得尺寸仅为内容区尺寸，不包括边框和padding尺寸。所以要加上边框和padding尺寸偏移才能获取正确的frame尺寸
	m_frame->resize(rect().width() + offset, rect().height() + m_titleBar->rect().height() + offset);

	m_frame->setWindowModality(Qt::ApplicationModal);//设置为Qt::ApplicationModal，否则弹出的模态对话框会在任务栏显示
	//m_frame->setWindowFlags(m_frame->windowFlags() | Qt::Tool);//所以加上Qt::Tool标志使其不出现在任务栏上

	m_frame->show();
	m_frame->raise();
	m_frame->activateWindow();

	int ret = QDialog::exec();
	m_frame->close();

	return ret;
}

void QssDialog::setWindowTitle( QString title )
{
	m_frame->setWindowTitle(title);
	m_titleBar->setTitle(title);
}

void QssDialog::setWindowIcon( QIcon icon )
{
	m_frame->setWindowIcon(icon);
	m_titleBar->setIcon(icon);
}

/** dialog仅在阴影边框才能调节尺寸大小 */
void QssDialog::onMouseMoveEvent( QMouseEvent * ev )
{
	if (m_mousePressedInBorder)
	{
		int x = ev->globalPos().x();
		int y = ev->globalPos().y();

		int dx = x - m_pos.x();
		int dy = y - m_pos.y();

		if ((m_left || m_right) && qAbs(dx) < 5)
			return;
		if ((m_top || m_bottom) && qAbs(dy) < 5)
			return;
		if (m_left && dx > 0 && m_rect.width() <= m_frame->minimumWidth())
			return ;
		if (m_top && dy > 0 && m_rect.height() <= m_frame->minimumHeight())
			return;

		QRect rc = m_rect;
		if (m_left)
			rc.setLeft(rc.left() + dx);
		if (m_right)
			rc.setRight(rc.right() + dx);
		if (m_top)
			rc.setTop(rc.top() + dy);
		if (m_bottom)
			rc.setBottom(rc.bottom() + dy);

		m_frame->setGeometry(rc);
		m_rect = rc;
		m_pos = ev->globalPos();
	}
	else
	{
		int x = ev->x() + QSSDIALOG_SHADOW_WIDTH - 2;
		int y = ev->y() + QSSDIALOG_SHADOW_WIDTH - 2;
		
		QRect rc = m_frame->rect();
		m_left = qAbs(x - rc.left()) <= 5;
		m_right = qAbs(x - rc.right()) <= 5;
		m_top = qAbs(y - rc.top()) <= 5;
		m_bottom = qAbs(y - rc.bottom()) <= 5;

		if ((m_left && m_top) || (m_right && m_bottom))
			m_frame->setCursor(Qt::SizeFDiagCursor);
		else if ((m_right && m_top) || (m_left && m_bottom))
			m_frame->setCursor(Qt::SizeBDiagCursor);
		else if ((m_left && !m_top && !m_bottom) || (m_right && !m_top && !m_bottom))
			m_frame->setCursor(Qt::SizeHorCursor);
		else if ((m_top && !m_right && !m_left) || (m_bottom && !m_right && !m_left))
			m_frame->setCursor(Qt::SizeVerCursor);
		else
			m_frame->setCursor(Qt::ArrowCursor);
	}
}

void QssDialog::onMousePressEvent( QMouseEvent * ev )
{
	m_pos = ev->globalPos();
	m_rect = m_frame->geometry();
	if(m_left || m_right || m_top || m_bottom)
	{
		m_mousePressedInBorder = ev->button() == Qt::LeftButton;
		//qDebug() << "mousePressed pressed in border";
	}
}

void QssDialog::onMouseReleaseEvent( QMouseEvent * ev )
{
	m_mousePressedInBorder = false;
	//qDebug() << "mousePressed release in border";
}

bool QssDialog::eventFilter( QObject * obj, QEvent * ev )
{
	if (obj == m_frame)
	{
		if (ev->type() == QEvent::MouseMove)
		{
			QMouseEvent * mouseEv = dynamic_cast<QMouseEvent *>(ev);
			if (ev)
			{
				onMouseMoveEvent(mouseEv);
				return true;
			}
		}
		else if (ev->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent * mouseEv = dynamic_cast<QMouseEvent *>(ev);
			if (ev)
			{
				onMousePressEvent(mouseEv);
				return true;
			}
		}
		else if (ev->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent * mouseEv = dynamic_cast<QMouseEvent *>(ev);
			if (ev)
			{
				onMouseReleaseEvent(mouseEv);
				return true;
			}
		}/** 绘制阴影边框 */
		else if (ev->type() == QEvent::Paint)
		{
			const int shadowWidth = QSSDIALOG_SHADOW_WIDTH;
			const int boderWidht = QSSDIALOG_BODER_WIDTH;

			QPainter paiter(m_frame);

			QColor colorBorder(0xaa,0xaa,0xaa);
			paiter.setPen(colorBorder);

			int boderOffset = shadowWidth + boderWidht - 1;
			paiter.drawLine(boderOffset,boderOffset,m_frame->width() - boderOffset - 1, boderOffset);//top
			paiter.drawLine(boderOffset,m_frame->height() - boderOffset - 1,m_frame->width() - boderOffset - 1, m_frame->height() - boderOffset - 1);//bottom

			paiter.drawLine(boderOffset,boderOffset,boderOffset,m_frame->height() - boderOffset - 1);//left
			paiter.drawLine(m_frame->width() - boderOffset - 1,boderOffset,m_frame->width() - boderOffset - 1,m_frame->height() - boderOffset - 1);//right

			QColor colorShadow(0xaa,0xaa,0xaa);
			for (int i = 0; i < shadowWidth; i++)
			{
				colorShadow.setAlpha(100*cos(1.5707963*i/(shadowWidth - 1)));
				paiter.setPen(colorShadow);
				paiter.drawRect(boderOffset + i, boderOffset + i, 
					m_frame->width() - 2*shadowWidth , 
					m_frame->height() - 2*shadowWidth );
			}
		}
		else if (ev->type() == QEvent::Show)
		{
			QRect rc = m_frame->rect(), parentRc;
			if (m_parent)/**有父控件时居父控件中 */
			{
				/** 父控件坐标转为屏幕坐标 */
				QPoint pt = m_parent->mapToGlobal(QPoint(0,0));
				parentRc =m_parent->rect();
				parentRc.translate(pt);
			}
			else/**无父控件时居屏幕中 */
				parentRc = m_rcValid;

			/** m_frame总是无父控件的，所以要相对屏幕坐标移动，坐标不能为负 */
			int x = parentRc.left() + (parentRc.width() - rc.width())*0.5;x = x <= 0?1:x;
			int y = parentRc.top() + (parentRc.height() - rc.height())*0.5;y = y <= 0?1:y;
			m_frame->move(x,y);
			
 			/** 属性动画 */
 			QPropertyAnimation* aniSize = new QPropertyAnimation(m_frame,"geometry");
 			aniSize->setDuration(200);
 			aniSize->setKeyValueAt(0, QRect(x,y,0,0));
 			aniSize->setKeyValueAt(0.5, QRect(x ,y,rc.width() + 20,rc.height() + 30));
 			aniSize->setKeyValueAt(1 , QRect(x,y,rc.width(),rc.height()));
 			//aniSize->setEasingCurve(QEasingCurve::InOutBack);//缓和曲线回缩还是太快了
 
 			QPropertyAnimation* aniOpacity = new QPropertyAnimation(m_frame,"windowOpacity");
 			aniOpacity->setDuration(200);
 			aniOpacity->setStartValue(0);
 			aniOpacity->setEndValue(1);
 
 			QParallelAnimationGroup* aniGroup = new QParallelAnimationGroup(m_frame);
 			aniGroup->addAnimation(aniSize);
 			aniGroup->addAnimation(aniOpacity);
 			
 			/** 显示动画开始，该函数不阻塞。动画播放完成后删除 */
 			aniGroup->start(QAbstractAnimation::DeleteWhenStopped);

		}
		else if (ev->type() == QEvent::Close)
		{
			close();
		}
	}
	else if (obj == m_titleBar)
	{
		if (ev->type() == QEvent::Enter)
		{
			m_left = false;m_right = false; m_top = false; m_bottom = false;
			if (m_frame->cursor().shape() != Qt::ArrowCursor)
				m_frame->setCursor(Qt::ArrowCursor);
		}
	}
	else if (obj == this)
	{
		if (ev->type() == QEvent::Enter)
		{
			m_left = false;m_right = false; m_top = false; m_bottom = false;
			if (m_frame->cursor().shape() != Qt::ArrowCursor)
				m_frame->setCursor(Qt::ArrowCursor);
		}
		else if (ev->type() == QEvent::Hide)
		{
			m_frame->hide();
			return true;
		}
	}

	return QDialog::eventFilter(obj, ev);
}

QssMessageBox::QssMessageBox( Icon icon, const QString &title, const QString &text, StandardButtons buttons /*= NoButton*/, QWidget *parent /*= 0*/, Qt::WindowFlags flags /*= Qt::Widget | Qt::FramelessWindowHint*/ )
:QMessageBox(icon, title, text, buttons, 0, flags),m_parent(parent)
{
	m_rcValid = QApplication::desktop()->availableGeometry(); 

	m_frame = new QFrame;
	m_frame->setObjectName("messagebox");//设置控件名称，方便css指定样式
	m_frame->setAttribute(Qt::WA_TranslucentBackground);/** 设置透明背景，padding出阴影边框绘制控件 */
	m_frame->setWindowFlags(Qt::Dialog |Qt::FramelessWindowHint| Qt::WindowSystemMenuHint);
	m_frame->setMouseTracking(true);
	m_frame->installEventFilter(this);

	m_frame->setWindowTitle(title);
	m_frame->setWindowIcon(style()->standardIcon((QStyle::StandardPixmap)(icon + 8)));
	m_frame->setWindowModality(Qt::ApplicationModal);

	m_titleBar = new QssTtitleBar(m_frame, QssTtitleBar::QTitleBar_Type_MessageBox);
	m_titleBar->installEventFilter(this);

	m_titleBar->setTitle(title);
	m_titleBar->setIcon(style()->standardIcon((QStyle::StandardPixmap)(icon + 8)));

	QVBoxLayout* vbox = new QVBoxLayout(m_frame);
	vbox->setMargin(0);
	vbox->setSpacing(0);
	vbox->addWidget(m_titleBar);
	vbox->addWidget(this);

	installEventFilter(this);
}

QssMessageBox::QssMessageBox( QWidget *parent /*= 0*/ )
:QMessageBox(parent),m_parent(parent)
{
	m_rcValid = QApplication::desktop()->availableGeometry(); 

	m_frame = new QFrame;
	m_frame->setObjectName("messagebox");//设置控件名称，方便css指定样式
	m_frame->setAttribute(Qt::WA_TranslucentBackground);/** 设置透明背景，padding出阴影边框绘制控件 */
	m_frame->setWindowFlags(Qt::FramelessWindowHint| Qt::WindowSystemMenuHint);
	m_frame->setMouseTracking(true);
	m_frame->installEventFilter(this);
	m_frame->setWindowModality(Qt::ApplicationModal);

	m_titleBar = new QssTtitleBar(m_frame, QssTtitleBar::QTitleBar_Type_MessageBox);
	m_titleBar->installEventFilter(this);

	QVBoxLayout* vbox = new QVBoxLayout(m_frame);
	vbox->setMargin(0);
	vbox->setSpacing(0);
	vbox->addWidget(m_titleBar);
	vbox->addWidget(this);

	installEventFilter(this);
}

QssMessageBox::~QssMessageBox()
{
	m_frame->deleteLater();
}

bool QssMessageBox::eventFilter( QObject * obj, QEvent * ev )
{
	if (obj == m_frame)
	{
		if (ev->type() == QEvent::Paint)
		{
			const int shadowWidth = QSSDIALOG_SHADOW_WIDTH;
			const int boderWidht = QSSDIALOG_BODER_WIDTH;

			QPainter paiter(m_frame);

			QColor colorBorder(0xaa,0xaa,0xaa);
			paiter.setPen(colorBorder);

			int boderOffset = shadowWidth + boderWidht - 1;
			paiter.drawLine(boderOffset,boderOffset,m_frame->width() - boderOffset - 1, boderOffset);//top
			paiter.drawLine(boderOffset,m_frame->height() - boderOffset - 1,m_frame->width() - boderOffset - 1, m_frame->height() - boderOffset - 1);//bottom

			paiter.drawLine(boderOffset,boderOffset,boderOffset,m_frame->height() - boderOffset - 1);//left
			paiter.drawLine(m_frame->width() - boderOffset - 1,boderOffset,m_frame->width() - boderOffset - 1,m_frame->height() - boderOffset - 1);//right

			QColor colorShadow(0xaa,0xaa,0xaa);
			for (int i = 0; i < shadowWidth; i++)
			{
				colorShadow.setAlpha(100*cos(1.5707963*i/(shadowWidth - 1)));
				paiter.setPen(colorShadow);
				paiter.drawRect(boderOffset + i, boderOffset + i, 
					m_frame->width() - 2*shadowWidth , 
					m_frame->height() - 2*shadowWidth );
			}
		}
		else if (ev->type() == QEvent::Show)
		{
			QRect rc = m_frame->rect(), parentRc;
			if (m_parent)/**有父控件时居父控件中 */
			{
				QPoint pt = m_parent->mapToGlobal(QPoint(0,0));
				parentRc =m_parent->rect();
				parentRc.translate(pt);
			}
			else/**无父控件时居屏幕中 */
				parentRc = m_rcValid;
			/** m_frame总是无父控件的，所以要相对屏幕坐标移动，坐标不能为负 */
			int x = parentRc.left() + (parentRc.width() - rc.width())*0.5;x = x <= 0?1:x;
			int y = parentRc.top() + (parentRc.height() - rc.height())*0.5;y = y <= 0?1:y;
			m_frame->move(x,y);

			/** 属性动画 */
			QPropertyAnimation* aniSize = new QPropertyAnimation(m_frame,"geometry");
			aniSize->setDuration(200);
			aniSize->setKeyValueAt(0, QRect(x,y,0,0));
			aniSize->setKeyValueAt(0.5, QRect(x ,y,rc.width() + 10,rc.height() + 15));
			aniSize->setKeyValueAt(1 , QRect(x,y,rc.width(),rc.height()));
			//aniSize->setEasingCurve(QEasingCurve::InOutBack);//缓和曲线回缩还是太快了

			QPropertyAnimation* aniOpacity = new QPropertyAnimation(m_frame,"windowOpacity");
			aniOpacity->setDuration(200);
			aniOpacity->setStartValue(0);
			aniOpacity->setEndValue(1);

			QParallelAnimationGroup* aniGroup = new QParallelAnimationGroup(m_frame);
			aniGroup->addAnimation(aniSize);
			aniGroup->addAnimation(aniOpacity);

			/** 显示动画开始，该函数不阻塞。动画播放完成后删除 */
			aniGroup->start(QAbstractAnimation::DeleteWhenStopped);

		}
		else if (ev->type() == QEvent::Close)
		{
			close();
			m_titleBar->close();
		}
	}
	else if(obj == this)
	{
		if (ev->type() == QEvent::Resize)
		{
			/* 解决QMessageBox自动适配内容调整宽度时，frame未调整宽度导致frame部分背景透明的bug */
			frame()->setFixedWidth(size().width() + 2*QSSDIALOG_SHADOW_WIDTH);
		}
	}

	return QDialog::eventFilter(obj, ev);
}

QMessageBox::StandardButton QssMessageBox::tips( const QString & text, QWidget* parent /*= 0*/, const QString & title /*= QString::fromLocal8Bit("提示")*/, StandardButtons btn /*= QMessageBox::Ok*/ )
{
	QssMessageBox box(QMessageBox::Information, title, "\n" + text/* + "\n"*/, btn, parent);
	box.setDefaultButton(QMessageBox::Ok);

	box.frame()->show();
	box.frame()->raise();
	box.frame()->activateWindow();

	QMessageBox::StandardButton ret = (QMessageBox::StandardButton)box.exec();
	box.frame()->close();

	return ret;
}

QMessageBox::StandardButton QssMessageBox::warn( const QString & text, QWidget* parent /*= 0*/, const QString & title /*= QString::fromLocal8Bit("警告")*/, StandardButtons btn /*= QMessageBox::Ok*/ )
{
	QssMessageBox box(QMessageBox::Warning, title, "\n" + text/* + "\n"*/ , btn, parent);
	box.setDefaultButton(QMessageBox::Ok);

	box.frame()->show();
	box.frame()->raise();
	box.frame()->activateWindow();

	QMessageBox::StandardButton ret = (QMessageBox::StandardButton)box.exec();
	box.frame()->close();

	return ret;
}

QMessageBox::StandardButton QssMessageBox::error( const QString & text, QWidget* parent /*= 0*/, const QString & title /*= QString::fromLocal8Bit("错误")*/, StandardButtons btn /*= QMessageBox::Ok*/ )
{
	QssMessageBox box(QMessageBox::Critical, title, "\n" + text/* + "\n"*/, btn, parent);
	box.setDefaultButton(QMessageBox::Ok);

	box.frame()->show();
	box.frame()->raise();
	box.frame()->activateWindow();

	QMessageBox::StandardButton ret = (QMessageBox::StandardButton)box.exec();
	box.frame()->close();

	return ret;
}

QMessageBox::StandardButton QssMessageBox::ask( const QString & text, QWidget* parent /*= 0*/, const QString & title /*= QString::fromLocal8Bit("询问")*/, StandardButtons btn /*= QMessageBox::Yes | QMessageBox::No*/ )
{
	QssMessageBox box(QMessageBox::Question, title, "\n" + text/* + "\n"*/, btn, parent);
	box.setDefaultButton(QMessageBox::Yes);

	box.frame()->show();
	box.frame()->raise();
	box.frame()->activateWindow();

	QMessageBox::StandardButton ret = (QMessageBox::StandardButton)box.exec();
	box.frame()->close();

	return ret;
}


QMessageBox::StandardButton QssMessageBox::regard( const QString & text, QWidget* parent /*= 0*/, const QString & title /*= QString::fromLocal8Bit("关于")*/ )
{
	QssMessageBox box(parent);

	QIcon icon = QApplication::windowIcon();
	QSize size = icon.actualSize(QSize(64, 64));
	box.setIconPixmap(icon.pixmap(size));
	box.setDefaultButton(QMessageBox::Ok);

	box.frame()->setWindowIcon(icon);
	box.titleBar()->setIcon(icon);

	box.frame()->setWindowTitle(title);
	box.titleBar()->setTitle(title);

	box.setInformativeText(text);

	box.frame()->show();
	box.frame()->raise();
	box.frame()->activateWindow();

	QMessageBox::StandardButton ret = (QMessageBox::StandardButton)box.exec();
	box.frame()->close();

	return ret;
}

QMessageBox::StandardButton QssMessageBox::regard( const QString & text, QIcon icon, QWidget* parent /*= 0*/, const QString & title /*= QString::fromLocal8Bit("关于")*/ )
{
	QssMessageBox box(parent);
	QSize size = icon.actualSize(QSize(64, 64));
	box.setIconPixmap(icon.pixmap(size));
	box.setDefaultButton(QMessageBox::Ok);
	
	box.frame()->setWindowIcon(icon);
	box.titleBar()->setIcon(icon);

	box.frame()->setWindowTitle(title);
	box.titleBar()->setTitle(title);

	box.setInformativeText(text);

	box.frame()->show();
	box.frame()->raise();
	box.frame()->activateWindow();

	QMessageBox::StandardButton ret = (QMessageBox::StandardButton)box.exec();
	box.frame()->close();

	return ret;
}

QMessageBox::StandardButton QssMessageBox::regardQt(QWidget* parent /*= 0*/, const QString & title /*= QString::fromLocal8Bit("关于Qt")*/ )
{
	QString translatedTextAboutQtCaption;
	translatedTextAboutQtCaption = QMessageBox::tr(
		"<h3>About Qt</h3>"
		"<p>This program uses Qt version %1.</p>"
		).arg(QLatin1String(QT_VERSION_STR));
	QString translatedTextAboutQtText;
	translatedTextAboutQtText = QMessageBox::tr(
		"<p>Qt is a C++ toolkit for cross-platform application "
		"development.</p>"
		"<p>Qt provides single-source portability across MS&nbsp;Windows, "
		"Mac&nbsp;OS&nbsp;X, Linux, and all major commercial Unix variants. "
		"Qt is also available for embedded devices as Qt for Embedded Linux "
		"and Qt for Windows CE.</p>"
		"<p>Qt is available under three different licensing options designed "
		"to accommodate the needs of our various users.</p>"
		"<p>Qt licensed under our commercial license agreement is appropriate "
		"for development of proprietary/commercial software where you do not "
		"want to share any source code with third parties or otherwise cannot "
		"comply with the terms of the GNU LGPL version 2.1 or GNU GPL version "
		"3.0.</p>"
		"<p>Qt licensed under the GNU LGPL version 2.1 is appropriate for the "
		"development of Qt applications (proprietary or open source) provided "
		"you can comply with the terms and conditions of the GNU LGPL version "
		"2.1.</p>"
		"<p>Qt licensed under the GNU General Public License version 3.0 is "
		"appropriate for the development of Qt applications where you wish to "
		"use such applications in combination with software subject to the "
		"terms of the GNU GPL version 3.0 or where you are otherwise willing "
		"to comply with the terms of the GNU GPL version 3.0.</p>"
		"<p>Please see <a href=\"http://qt.nokia.com/products/licensing\">qt.nokia.com/products/licensing</a> "
		"for an overview of Qt licensing.</p>"
		"<p>Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).</p>"
		"<p>Qt is a Nokia product. See <a href=\"http://qt.nokia.com/\">qt.nokia.com</a> "
		"for more information.</p>"
		);

	QPixmap pm(QMessageBox::tr(":/trolltech/qmessagebox/images/qtlogo-64.png"));

	QssMessageBox box(parent);
	box.setWindowTitle(title.isEmpty() ? QString::fromLocal8Bit("关于Qt") : title);
	box.setText(translatedTextAboutQtCaption);
	box.setInformativeText(translatedTextAboutQtText);
	if (!pm.isNull())
		box.setIconPixmap(pm);
	box.setDefaultButton(QMessageBox::Ok);

	box.frame()->setWindowIcon(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"));
	box.frame()->setWindowTitle(title.isEmpty() ? QString::fromLocal8Bit("关于Qt") : title);
	box.titleBar()->setIcon(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"));
	box.titleBar()->setTitle(title.isEmpty() ? QString::fromLocal8Bit("关于Qt") : title);

	box.frame()->show();
	box.frame()->raise();
	box.frame()->activateWindow();

	QMessageBox::StandardButton ret = (QMessageBox::StandardButton)box.exec();
	box.frame()->close();

	return ret;
}


QMessageBox::StandardButton QssMessageBox::information( QWidget *parent, const QString &title, const QString &text, StandardButtons buttons /*= Ok*/, StandardButton defaultButton /*= NoButton*/ )
{
	return QssMessageBox::tips(text, parent, title, buttons);
}

QMessageBox::StandardButton QssMessageBox::question( QWidget *parent, const QString &title, const QString &text, StandardButtons buttons /*= QMessageBox::Ok*/, StandardButton defaultButton /*= QMessageBox::NoButton*/ )
{
	return QssMessageBox::ask(text, parent, title, buttons);
}

QMessageBox::StandardButton QssMessageBox::warning( QWidget *parent, const QString &title, const QString &text, StandardButtons buttons /*= QMessageBox::Ok*/, StandardButton defaultButton /*= QMessageBox::NoButton*/ )
{
	return QssMessageBox::warn(text, parent, title, buttons);
}

QMessageBox::StandardButton QssMessageBox::critical( QWidget *parent, const QString &title, const QString &text, StandardButtons buttons /*= QMessageBox::Ok*/, StandardButton defaultButton /*= QMessageBox::NoButton*/ )
{
	return QssMessageBox::error(text, parent, title, buttons);
}

void QssMessageBox::about( QWidget *parent, const QString &title, const QString &text )
{
	QssMessageBox::regard(text, parent, title);
}

void QssMessageBox::about( QWidget *parent, const QString &title, const QString &text, QIcon icon )
{
	QssMessageBox::regard(text, icon, parent, title);
}

void QssMessageBox::aboutQt( QWidget *parent, const QString &title /*= QString()*/ )
{
	QssMessageBox::regardQt(parent, title);
}

