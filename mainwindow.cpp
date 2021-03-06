#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cubie.h"
#include "connectdialog.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->statusBar->showMessage("Нет соединения");
	keyState[0]=false; // вверх
	keyState[1]=false; // вниз
	keyState[2]=false; // вправо
	keyState[3]=false; // влево

	throttleUpKeyState = false;
	throttleDownKeyState = false;
	yawLeftKeyState = false;
	yawRightKeyState = false;

	connected=false;
	/*
	ui->label_up_blue->hide();
	ui->label_right_blue->hide();
	ui->label_down_blue->hide();
	ui->label_left_blue->hide();
	ui->label_up_right_blue->hide();
	ui->label_up_left_blue->hide();
	ui->label_down_left_blue->hide();
	ui->label_down_right_blue->hide();
	*/
	this->sendedPackets = 1;
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	//
	timer->start(20);
	QTimer *graphUpdater = new QTimer(this);
	connect(graphUpdater, SIGNAL(timeout()), this, SLOT(graphUpdate()));

	graphUpdater->start(100);
	graphInit();

}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::graphInit(){

	x.resize(100);
	pitchData.resize(100);
	rollData.resize(100);
	yawData.resize(100);

	for (int i=0; i<100; ++i)
	{
		x[i] = i; // x goes from 0 to 99
		pitchData[i] = 0;
		rollData[i] = 0;
		yawData[i] = 0;
	}

	//======================================Pitch==========
	ui->pitch_plot->addGraph();
	ui->pitch_plot->graph(0)->setData(x,pitchData);
	//настраиваем оси координат
	ui->pitch_plot->xAxis->setLabel("");
	ui->pitch_plot->yAxis->setLabel("");
	ui->pitch_plot->xAxis->setRange(0,100); // По оси абсцисс 100 значений
	ui->pitch_plot->yAxis->setRange(-90,90); // По оси ординат диапазон от -90 до 90 градусов
	ui->pitch_plot->xAxis->setVisible(false);
	ui->pitch_plot->yAxis->setVisible(true);
	QFont font_helvetica("Helvetica");
	QVector<double> yTicks(7);
	for (int i=0; i<7; ++i)
	{
		yTicks[i]=i*30 - 90;
	}

	ui->pitch_plot->yAxis->setLabelFont(font_helvetica);
	// ui->pitch_plot->yAxis->setLabel
	ui->pitch_plot->yAxis->setPadding(11);
	ui->pitch_plot->yAxis->setAutoTicks(false);
	ui->pitch_plot->yAxis->setTickVector(yTicks);
	ui->pitch_plot->setBackground(QBrush(QColor(239,235,231)));

	ui->pitch_plot->replot();
	//======================================Roll===========
	ui->roll_plot->addGraph();
	ui->roll_plot->graph(0)->setData(x,rollData);

	//настраиваем оси координат
	ui->roll_plot->xAxis->setLabel("");
	ui->roll_plot->yAxis->setLabel("");
	ui->roll_plot->xAxis->setRange(0,100); // По оси абсцисс 100 значений
	ui->roll_plot->yAxis->setRange(-90,90); // По оси ординат диапазон от -90 до 90 градусов
	ui->roll_plot->xAxis->setVisible(false);
	ui->roll_plot->yAxis->setVisible(true);
	ui->roll_plot->yAxis->setLabelFont(font_helvetica);
	// ui->roll_plot->yAxis->setLabel
	ui->roll_plot->yAxis->setPadding(11);
	ui->roll_plot->yAxis->setAutoTicks(false);
	ui->roll_plot->yAxis->setTickVector(yTicks);
	ui->roll_plot->setBackground(QBrush(QColor(239,235,231)));
	ui->roll_plot->replot();

	//=======================================Yaw=============
	ui->yaw_plot->addGraph();
	ui->yaw_plot->graph(0)->setData(x,yawData);

	//настраиваем оси координат
	ui->yaw_plot->xAxis->setLabel("");
	ui->yaw_plot->yAxis->setLabel("");

	ui->yaw_plot->xAxis->setRange(0,100); // По оси абсцисс 100 значений
	ui->yaw_plot->yAxis->setRange(-180,180); // По оси ординат диапазон от -90 до 90 градусов
	ui->yaw_plot->xAxis->setVisible(false);
	ui->yaw_plot->yAxis->setVisible(true);
	ui->yaw_plot->yAxis->setLabelFont(font_helvetica);
	// ui->roll_plot->yAxis->setLabel

	QVector<double> yTicksYaw(7);
	for (int i=0; i<7; ++i)
	{
		yTicksYaw[i]=i*60 - 180;
	}


	ui->yaw_plot->yAxis->setPadding(0);
	ui->yaw_plot->yAxis->setAutoTicks(false);
	ui->yaw_plot->yAxis->setTickVector(yTicksYaw);
	ui->yaw_plot->setBackground(QBrush(QColor(239,235,231)));
	ui->yaw_plot->replot();
}

void MainWindow::graphUpdate(){
	//========================Pitch Update============
	pitchData.removeFirst();
	pitchData.append(cubie->getFeedbackPitch());
	ui->pitch_plot->graph(0)->setData(x,pitchData);
	ui->pitch_plot->replot();
	ui->displayGL->setPitchAngle(cubie->getFeedbackPitch());

	//========================Roll Update============
	rollData.removeFirst();
	rollData.append(cubie->getFeedbackRoll());
	ui->roll_plot->graph(0)->setData(x,rollData);
	ui->roll_plot->replot();
	ui->displayGL->setRollAngle(cubie->getFeedbackRoll());
	//========================Yaw Update============
	yawData.removeFirst();
	yawData.append(cubie->getFeedbackYaw());
	ui->yaw_plot->graph(0)->setData(x,yawData);
	ui->yaw_plot->replot();
	ui->displayGL->setYawAngle(cubie->getFeedbackYaw());

	//========================Throttle Update========
	ui->feedback_throttle->setValue(cubie->getFeedbackThrottle());
	ui->feedback_throttle_label->setText(QString::number(cubie->getFeedbackThrottle()));

	ui->displayGL->updateGL();

	return;
}

void MainWindow::feedbackUpdate(){
	float pitch = cubie->getFeedbackPitch();
	float roll = cubie->getFeedbackRoll();
	float yaw = cubie->getFeedbackYaw();

	ui->pitch_info_label->setText(QString::number(pitch));
	ui->roll_info_label->setText(QString::number(roll));
	ui->yaw_info_label->setText(QString::number(yaw));
	graphUpdate();

}

void MainWindow::update(){


	//Разбор крена
	if(keyState[2] && !keyState[3]) {
		roll_right();
		if (connected){
			cubie->sendVar("roll",ui->label_roll_control->text());
		}
	}
	else if(keyState[3] && !keyState[2]) {
		roll_left();
		if (connected){
			cubie->sendVar("roll",ui->label_roll_control->text());
		}
	}
	else if(keyState[2] && keyState[3]) {

	} else if (ui->label_roll_control->text().toInt()!=0) {
		roll_center();
		if (connected){
			cubie->sendVar("roll",ui->label_roll_control->text());
		}
	}
	//Разбор тангажа
	if(keyState[0] && !keyState[1]) {
		pitch_up();
		if (connected){
			cubie->sendVar("pitch",ui->label_pitch_control->text());
		}
	}
	else if(keyState[1] && !keyState[0]) {
		pitch_down();
		if (connected){
			cubie->sendVar("pitch",ui->label_pitch_control->text());
		}
	}
	else if(keyState[0] && keyState[1]) {

	} else if(ui->label_pitch_control->text().toInt()!=0) {
		pitch_center();
		if (connected){
			cubie->sendVar("pitch",ui->label_pitch_control->text());
		}
	}

	// разбор клавиш тяги
	if(throttleUpKeyState && !throttleDownKeyState ){
		throttle_up();
		if (connected){
			char throttle_text[4];
			sprintf(throttle_text,"%d",ui->throttleSlider->value());
			cubie->sendVar("throttle",throttle_text);
		}
	} else if(throttleDownKeyState && !throttleUpKeyState){
		throttle_down();
		if (connected){
			char throttle_text[4];
			sprintf(throttle_text,"%d",ui->throttleSlider->value());
			cubie->sendVar("throttle",throttle_text);
		}
	}

	// разбор клавиш рыскания
	char yaw_text[4];
	if(yawLeftKeyState && !yawRightKeyState){
		yaw_left();
		if (connected){
			sprintf(yaw_text,"%d",ui->yawSlider->value());
			cubie->sendVar("yaw",yaw_text);
		}
	} else if(yawRightKeyState && !yawLeftKeyState){
		yaw_right();
		if (connected){
			sprintf(yaw_text,"%d",ui->yawSlider->value());
			cubie->sendVar("yaw",yaw_text);
		}
	} else if(yawRightKeyState && yawLeftKeyState){
	// нажаты обе клавиши, ничего не делаем
	}else if(ui->yawSlider->value()!=0){
		yaw_center();
		if (connected){
			sprintf(yaw_text,"%d",ui->yawSlider->value());
			cubie->sendVar("yaw",yaw_text);
		}
	}

	if(connected){
		// cubie->sendVar("testd","yuhhu");
		ui->statusBar->showMessage("Отправлено пакетов: " + QString::number(sendedPackets));
	}
}

void MainWindow::keyPressEvent(QKeyEvent *keyEvent)
{
	if(keyEvent->key()==Qt::Key_Up)
	{
		keyState[0]=true;
	}
	else if(keyEvent->key()==Qt::Key_Down)
	{
		keyState[1]=true;
	}
	else if(keyEvent->key()==Qt::Key_Right)
	{
		keyState[2]=true;
	}
	else if(keyEvent->key()==Qt::Key_Left)
	{
		keyState[3]=true;
	}
	else if(keyEvent->key()==Qt::Key_W)
	{
		throttleUpKeyState=true;
	}
	else if(keyEvent->key()==Qt::Key_S)
	{
		throttleDownKeyState=true;
	}
	else if(keyEvent->key()==Qt::Key_C)
	{
		yawLeftKeyState=true;
	}
	else if(keyEvent->key()==Qt::Key_V)
	{
		yawRightKeyState=true;
	}
}

void MainWindow::keyReleaseEvent(QKeyEvent *keyEvent)
{
	if(keyEvent->key()==Qt::Key_Up)
	{
		keyState[0]=false;
	}
	else if(keyEvent->key()==Qt::Key_Down)
	{
		keyState[1]=false;
	}
	else if(keyEvent->key()==Qt::Key_Right)
	{
		keyState[2]=false;
	}
		else if(keyEvent->key()==Qt::Key_Left)
	{
		keyState[3]=false;
	}
	else if(keyEvent->key()==Qt::Key_W)
	{
		throttleUpKeyState=false;
	}
		else if(keyEvent->key()==Qt::Key_S)
	{
		throttleDownKeyState=false;
	}
	else if(keyEvent->key()==Qt::Key_C)
	{
		yawLeftKeyState=false;
	}
	else if(keyEvent->key()==Qt::Key_V)
	{
		yawRightKeyState=false;
	}
}

void MainWindow::on_action_connect_triggered()
{
	if(connected){
		cubie->disconnectFromCubie();
		QMessageBox::information(0, "Information", "Отключено от " + cDialog.getIp() + ":" + QString::number(cDialog.getPort()));
		ui->statusBar->showMessage("Соединение разорвано");
		ui->label_2->setText("Нет соединения");
		ui->action_connect->setText("Подключится к ...");
		connected=false;
	} else{
		if (cDialog.exec() == QDialog::Accepted) {
			this->cubie = new Cubie(cDialog.getIp(), 26000, cDialog.getPort());
			this->cubie->connectToCubie();
			connect(cubie, SIGNAL(packetSent()), this, SLOT(packetSent()));
			qDebug() << "test";
			connect(cubie, SIGNAL(connectionEstablished()), this, SLOT(connectionEstablished()));
		}
	}
	//Cubie *cubie = new Cubie("91.123.151.230", 26000);
}
void MainWindow::connectionEstablished() {
	QMessageBox::information(0, "Information", "Соединено c " + cDialog.getIp() + ":" + QString::number(cDialog.getPort()));
	ui->statusBar->showMessage("Соединение установлено");
	ui->label_2->setText("Соединено c " + cDialog.getIp() + ":" + QString::number(cDialog.getPort()));
	ui->action_connect->setText("Разъеденить");
	//this->showFullScreen();
	connected=true;
	cubie->sendTest();
	connect(cubie, SIGNAL(dataReady()), this, SLOT(feedbackUpdate()));
	connect(cubie, SIGNAL(newFeedbackConnection()), this, SLOT(feedbackConnectionEstablished()));
}

void MainWindow::packetSent() {
	sendedPackets++;
}

void MainWindow::throttle_up(){
	int old_value =ui->throttleSlider->value();
	int new_value = old_value+ui->throttle_sesitivity_spinBox->value();
	ui->throttleSlider->setValue(new_value);
}

void MainWindow::throttle_down(){
	int old_value =ui->throttleSlider->value();
	int new_value = old_value-ui->throttle_sesitivity_spinBox->value();
	ui->throttleSlider->setValue(new_value);
}
void MainWindow::yaw_left(){
	int old= ui->yawSlider->value();
	int new_val = old-ui->yaw_sesitivity_spinBox->value();
	ui->yawSlider->setValue(new_val);
}
void MainWindow::yaw_right(){
	int old= ui->yawSlider->value();
	int new_val = old+ui->yaw_sesitivity_spinBox->value();
	ui->yawSlider->setValue(new_val);
}

void MainWindow::yaw_center(){
	int old= ui->yawSlider->value();
	int new_val=0;
	if(old>0){
		if (old >= ui->yaw_sesitivity_spinBox->value()){
			new_val = old-ui->yaw_sesitivity_spinBox->value();
		}
	}
	else if(old<0){
		if ((0-old) >= ui->yaw_sesitivity_spinBox->value()){
		new_val = old+ui->yaw_sesitivity_spinBox->value();
		}
	}
	ui->yawSlider->setValue(new_val);
}

void MainWindow::roll_left(){
	int old= ui->label_roll_control->text().toInt();
	int new_val = old-ui->pitch_roll_sesitivity_spinBox->value();
	if(new_val<-100) new_val=-100;
	ui->label_roll_control->setText(QString::number(new_val));

	int x = ui->label_circle_blue->geometry().x();
	int y = ui->label_circle_blue->geometry().y();
	QSize size = ui->label_circle_blue->geometry().size();

	int new_x = 68+ui->label_roll_control->text().toInt();

	ui->label_circle_blue->setGeometry(new_x,y, size.width(), size.height());

}
void MainWindow::roll_right(){
	int old= ui->label_roll_control->text().toInt();
	int new_val = old+ui->pitch_roll_sesitivity_spinBox->value();
	if(new_val>100) new_val=100;
	ui->label_roll_control->setText(QString::number(new_val));

	int x = ui->label_circle_blue->geometry().x();
	int y = ui->label_circle_blue->geometry().y();
	QSize size = ui->label_circle_blue->geometry().size();

	int new_x = 68+ui->label_roll_control->text().toInt();

	ui->label_circle_blue->setGeometry(new_x,y, size.width(), size.height());
}
void MainWindow::roll_center(){
	int old= ui->label_roll_control->text().toInt();
	int new_val=0;
	if(old>0){
		if (old >= ui->pitch_roll_sesitivity_spinBox->value()){
			new_val = old-ui->pitch_roll_sesitivity_spinBox->value();
		}
	}
	else if(old<0){
		if ((0-old) >= ui->pitch_roll_sesitivity_spinBox->value()){
		new_val = old+ui->pitch_roll_sesitivity_spinBox->value();
		}
	}
	ui->label_roll_control->setText(QString::number(new_val));

	int x = ui->label_circle_blue->geometry().x();
	int y = ui->label_circle_blue->geometry().y();
	QSize size = ui->label_circle_blue->geometry().size();

	int new_x = 68+ui->label_roll_control->text().toInt();

	ui->label_circle_blue->setGeometry(new_x,y, size.width(), size.height());
}

void MainWindow::pitch_up(){
	int old= ui->label_pitch_control->text().toInt();
	int new_val = old-ui->pitch_roll_sesitivity_spinBox->value();
	if(new_val<-100) new_val=-100;
	ui->label_pitch_control->setText(QString::number(new_val));
	int x = ui->label_circle_blue->geometry().x();
	int y = ui->label_circle_blue->geometry().y();
	QSize size = ui->label_circle_blue->geometry().size();

	int new_y = 68+ui->label_pitch_control->text().toInt();

	ui->label_circle_blue->setGeometry(x,new_y, size.width(), size.height());
}
void MainWindow::pitch_down(){
	int old= ui->label_pitch_control->text().toInt();
	int new_val = old+ui->pitch_roll_sesitivity_spinBox->value();
	if(new_val>100) new_val=100;
	ui->label_pitch_control->setText(QString::number(new_val));

	int x = ui->label_circle_blue->geometry().x();
	int y = ui->label_circle_blue->geometry().y();
	QSize size = ui->label_circle_blue->geometry().size();

	int new_y = 68+ui->label_pitch_control->text().toInt();

	ui->label_circle_blue->setGeometry(x,new_y, size.width(), size.height());
}
void MainWindow::pitch_center(){
	int old= ui->label_pitch_control->text().toInt();
	int new_val=0;
	if(old>0){
		if (old >= ui->pitch_roll_sesitivity_spinBox->value()){
			new_val = old-ui->pitch_roll_sesitivity_spinBox->value();
		}
	}
	else if(old<0){
		if ((0-old) >= ui->pitch_roll_sesitivity_spinBox->value()){
		new_val = old+ui->pitch_roll_sesitivity_spinBox->value();
		}
	}
	ui->label_pitch_control->setText(QString::number(new_val));


	int x = ui->label_circle_blue->geometry().x();
	int y = ui->label_circle_blue->geometry().y();
	QSize size = ui->label_circle_blue->geometry().size();

	int new_y = 68+ui->label_pitch_control->text().toInt();

	ui->label_circle_blue->setGeometry(x,new_y, size.width(), size.height());
}

void MainWindow::on_throttleSlider_valueChanged(int value)
{
	char text[4];
	sprintf(text, "%d", value);
	QString valText(text);
	ui->label_throttle_set->setText(valText);
}

void MainWindow::on_action_keys_triggered()
{
	kDialog.exec();
}



void MainWindow::on_P_SpinBox_valueChanged(const QString &arg1)
{
	//Посылаем коэффициенты
	if (connected){
		cubie->sendVar("p",QString::number(ui->P_SpinBox->value()*1000));
	}
}

void MainWindow::on_I_SpinBox_valueChanged(const QString &arg1)
{
	//Посылаем коэффициенты
	if (connected){
		cubie->sendVar("i",QString::number(ui->I_SpinBox->value()*1000));
	}
}

void MainWindow::on_D_SpinBox_valueChanged(const QString &arg1)
{
	//Посылаем коэффициенты
	if (connected){
		cubie->sendVar("d",QString::number(ui->D_SpinBox->value()*1000));
	}
}
void MainWindow::feedbackConnectionEstablished(){
	ui->feedbackConnectLabel->setText("Входящее соединение установлено");
}
