#include <QApplication>

/* prefix debugovacich zprav pro programatora */
#define PREFIX '$' 
#include "VIZ_mainWindow.h"

VIZ_mainWindow * msgCatcher;

void msgHandler(QtMsgType type, const char * msg)
{
	QString text(msg);
	/* zpravy s prefixem nejsou pro uzivatele */	
	bool prefixed = (text.at(0) == PREFIX);
	if (prefixed)
		text = text.remove(0,1);	/* odeber prvni znak */

	switch (type)
	{
		case QtDebugMsg:
			if (prefixed)
				msgCatcher->appendToLog(prefixed,text, Qt::cyan);
			else
				msgCatcher->appendToLog(prefixed,text, Qt::green);
		break;
		case QtWarningMsg:
			if (prefixed)
				msgCatcher->appendToLog(prefixed,text, Qt::red);
			else
				msgCatcher->appendToLog(prefixed,text, Qt::blue);
		break;
		case QtCriticalMsg:
			if (prefixed)
				msgCatcher->appendToLog(prefixed,text, Qt::yellow);
			else
				msgCatcher->appendToLog(prefixed,text, Qt::darkYellow);
		break;
		case QtFatalMsg:
			msgCatcher->appendToLog(false,text.prepend("FATAL "));
			msgCatcher->crash();
	}
}

int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	VIZ_mainWindow mainWindow;

	msgCatcher = &mainWindow;

	qInstallMsgHandler(msgHandler);

	mainWindow.show();
	return app.exec();
}
