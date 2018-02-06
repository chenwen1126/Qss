/**
*	@file:		QssTtitleBar.h 
*	@details:	css控件定制头文件，包括所有UI控件及自定义标题栏的主窗口，对话框及消息框
*	@author: 	chenwen(chenwen1126@tom.com)
*   @datetime:  2017-7-25
*   @history:   v1.0 first edition
*/
#ifndef QSS_H
#define QSS_H

#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QMessageBox>
#include <QFrame>

class QPushButton;
class QLabel;
class QMouseEvent;

class  QssTtitleBar : public QWidget
{
	Q_OBJECT

public:
	enum QTitleBar_Button
	{
		QTitleBar_Button_Min = 		0x00000001,
		QTitleBar_Button_Max = 		0x00000002,
		QTitleBar_Button_Restore = 	0x00000004,
		QTitleBar_Button_Close = 	0x00000008
	};

	enum QTitleBar_Type
	{
		QTitleBar_Type_MainWindow = QTitleBar_Button_Min | QTitleBar_Button_Max | QTitleBar_Button_Restore | QTitleBar_Button_Close,
		QTitleBar_Type_Dialog = QTitleBar_Button_Close,
		QTitleBar_Type_MessageBox = QTitleBar_Button_Close
	};

	QssTtitleBar(QWidget *parent, QTitleBar_Type type = QTitleBar_Type_MainWindow);
	~QssTtitleBar();

	void setTitle(QString title);
	void setIcon( QIcon icon);

	void setMaxOrRestore(bool val);
	bool maxOrRestore(){return m_maxOrRestore;}

	QRect& normalRect(){return m_rcNormal;}

private slots:
	void onMaxOrRestore();

protected:
	void paintEvent(QPaintEvent *);
	void mouseMoveEvent(QMouseEvent * ev);
	void mousePressEvent(QMouseEvent * ev);
	void mouseReleaseEvent(QMouseEvent * ev);

	bool eventFilter(QObject * obj, QEvent * ev);

private:
	QPushButton* m_closeBtn;
	QPushButton* m_maxBtn;
	QPushButton* m_restoreBtn;
	QPushButton* m_minBtn;
	QPushButton* m_iconBtn;

	QLabel* m_titlebarTitle;

	bool m_maxOrRestore;
	bool m_pressed;

	QPoint m_pressedPos;

	QRect m_rcValid;//桌面最大可用尺寸
	QRect m_rcNormal;//还原后窗口尺寸

	QTitleBar_Type m_type;
};

class  QssMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	QssMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~QssMainWindow();

	void show();
	void showMinimized();
	void showMaximized();
	void showFullScreen();
	void showNormal();

	void setWindowTitle( QString title );
	void setWindowIcon( QIcon icon );

	inline QssTtitleBar* titleBar(){return m_titleBar;}
	inline QFrame* frame(){return m_frame;}

	virtual bool eventFilter(QObject * obj, QEvent * ev);

private:
	void onMouseMoveEvent(QMouseEvent * ev);
	void onMousePressEvent(QMouseEvent * ev);
	void onMouseReleaseEvent(QMouseEvent * ev);

private:
	QFrame* m_frame;
	QssTtitleBar* m_titleBar;

	QRect m_rcValid;//桌面最大可用尺寸
	QRect m_rcNormal;//还原后窗口尺寸

	/**  边框调整大小相关 */
	QRect m_rect;
	QPoint m_pos;
	bool m_mousePressedInBoundy;
	bool m_left,m_right,m_top,m_bottom;

};

class  QssDialog : public QDialog
{
	Q_OBJECT

public:
	QssDialog(QWidget *parent);
	virtual ~QssDialog();

	void show();
	void raise();
	void activateWindow();

	int exec();

	void setWindowTitle( QString title );
	void setWindowIcon( QIcon icon );

	inline QssTtitleBar* titleBar(){return m_titleBar;}
	inline QFrame* frame(){return m_frame;}

	virtual bool eventFilter(QObject * obj, QEvent * ev);

private:
	void onMouseMoveEvent(QMouseEvent * ev);
	void onMousePressEvent(QMouseEvent * ev);
	void onMouseReleaseEvent(QMouseEvent * ev);

private:
	QFrame* m_frame;
	QssTtitleBar* m_titleBar;

	QRect m_rcValid;
	QWidget* m_parent;

	/**  边框调整大小相关 */
	QRect m_rect;
	QPoint m_pos;
	bool m_mousePressedInBorder;
	bool m_left,m_right,m_top,m_bottom;

};

class  QssMessageBox : public QMessageBox
{
	Q_OBJECT

public:
	explicit QssMessageBox(QWidget *parent = 0);
	QssMessageBox(Icon icon, const QString &title, const QString &text,
		StandardButtons buttons = NoButton, QWidget *parent = 0,
		Qt::WindowFlags flags = Qt::Widget | Qt::FramelessWindowHint);
	~QssMessageBox();

	inline QssTtitleBar* titleBar(){return m_titleBar;}
	inline QFrame* frame(){return m_frame;}

	/** 配合宏使用，更为简洁 */
	static QMessageBox::StandardButton tips(const QString & text, QWidget* parent = 0, 
		const QString & title = QString::fromLocal8Bit("提示"), StandardButtons btn = QMessageBox::Ok);
	static QMessageBox::StandardButton warn(const QString & text, QWidget* parent = 0, 
		const QString & title = QString::fromLocal8Bit("警告"), StandardButtons btn = QMessageBox::Ok);
	static QMessageBox::StandardButton error(const QString & text, QWidget* parent = 0, 
		const QString & title = QString::fromLocal8Bit("错误"), StandardButtons btn = QMessageBox::Ok);
	static QMessageBox::StandardButton ask(const QString & text, QWidget* parent = 0, 
		const QString & title = QString::fromLocal8Bit("询问"), StandardButtons btn = QMessageBox::Yes | QMessageBox::No);

	/** 覆盖定义，适配原有QMessageBox */
	static StandardButton information(QWidget *parent, const QString &title,
		const QString &text, StandardButtons buttons = Ok,
		StandardButton defaultButton = NoButton);
	static StandardButton question(QWidget *parent, const QString &title,
		const QString &text, StandardButtons buttons = QMessageBox::Ok,
		StandardButton defaultButton = QMessageBox::NoButton);
	static StandardButton warning(QWidget *parent, const QString &title,
		const QString &text, StandardButtons buttons = QMessageBox::Ok,
		StandardButton defaultButton = QMessageBox::NoButton);
	static StandardButton critical(QWidget *parent, const QString &title,
		const QString &text, StandardButtons buttons = QMessageBox::Ok,
		StandardButton defaultButton = QMessageBox::NoButton);

	static QMessageBox::StandardButton regard(const QString & text, QWidget* parent = 0,
		const QString & title = QString::fromLocal8Bit("关于"));
	static QMessageBox::StandardButton regard(const QString & text, QIcon icon, QWidget* parent = 0,
		const QString & title = QString::fromLocal8Bit("关于"));

	static QMessageBox::StandardButton regardQt(QWidget* parent = 0, 
		const QString & title = QString::fromLocal8Bit("关于Qt"));

	static void about(QWidget *parent, const QString &title, const QString &text);
	static void about(QWidget *parent, const QString &title, const QString &text, QIcon icon);

	static void aboutQt(QWidget *parent, const QString &title = QString());

	bool eventFilter(QObject * obj, QEvent * ev);

private:
	QFrame* m_frame;
	QssTtitleBar* m_titleBar;
	QWidget* m_parent;

	QRect m_rcValid;
};

#define tipBox(text)   	QssMessageBox::tips(text)
#define warnBox(text)  	QssMessageBox::warn(text)
#define errBox(text)	QssMessageBox::error(text)
#define askBox(text)   	QssMessageBox::ask(text)

#define aboutBox(title, text)	QssMessageBox::regard(text, 0,title)
#define aboutQtBox()			QssMessageBox::regardQt()

#endif // QSS_H
