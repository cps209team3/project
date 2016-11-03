#ifndef PAUSESCREEN_H
#define PAUSESCREEN_H

#include <QWidget>

namespace Ui {
class PauseScreen;
}

class PauseScreen : public QWidget
{
	Q_OBJECT

public:
	explicit PauseScreen(QWidget *parent = 0);
	~PauseScreen();

signals:
	void resumeClicked();

private slots:
	void on_btnResume_clicked();

	void on_btnHelp_clicked();

	void on_btnCheat_clicked();

	void on_btnTitle_clicked();

	void on_btnExit_clicked();

private:
	Ui::PauseScreen *ui;
};

#endif // PAUSESCREEN_H