#include <QApplication>
#include <QByteArray>

/* prefix debugovacich zprav pro programatora */
#define PREFIX '$' 
#include "VIZ_mainWindow.h"

VIZ_mainWindow * msgCatcher;

void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray text = msg.toLocal8Bit();

    /* prefixed messages are not supposed to be displayed in user output log */
    bool prefixed = text.startsWith(PREFIX);

    /* remove prefix */
    if (prefixed)
        text.remove(0,1);

    switch (type) {
        case QtDebugMsg:
            if (prefixed)
                fprintf(stderr, "Debug: %s (%s:%u, %s)\n", text.constData(), context.file, context.line, context.function);
            else
                msgCatcher->appendToLog(text, Qt::green);
            break;
        case QtInfoMsg:
            if (prefixed)
                fprintf(stderr, "Info: %s (%s:%u, %s)\n", text.constData(), context.file, context.line, context.function);
            else
                msgCatcher->appendToLog(text, Qt::blue);
            break;
        case QtWarningMsg:
            if (prefixed)
                fprintf(stderr, "Warning: %s (%s:%u, %s)\n", text.constData(), context.file, context.line, context.function);
            else
                msgCatcher->appendToLog(text, Qt::yellow);
            break;
        case QtCriticalMsg:
            if (prefixed)
                fprintf(stderr, "Critical: %s (%s:%u, %s)\n", text.constData(), context.file, context.line, context.function);
            else
                msgCatcher->appendToLog(text, Qt::red);
            break;
        case QtFatalMsg:
            if (prefixed)
                fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", text.constData(), context.file, context.line, context.function);
            else
                msgCatcher->appendToLog(text, Qt::darkMagenta);
            abort();
    }
}

int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	VIZ_mainWindow mainWindow;

	msgCatcher = &mainWindow;

    qInstallMessageHandler(msgHandler);

	mainWindow.show();
	return app.exec();
}
