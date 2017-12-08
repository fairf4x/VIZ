#ifndef CONVERTOR_H
#define CONVERTOR_H

#include "XMLHelpFunctions.h"

class QWidget;

class Convertor
{
	public:
		Convertor(QWidget * owner = 0);

		QDomDocument	readFromXML(QFile & input);
		void		writeToXML(const QDomDocument & doc, QFile & output);

	/*	QDomDocument	readDomainFromPDDL(QFile & input);
		QDomDocument	readProblemFromPDDL(QFile & input, QDomDocument domainDef); */

		void		writeDomainToPDDL(const QDomDocument & doc, QFile & output);
		void		writeProblemsToPDDL(const QDomDocument & doc, QString & targetDir);
	private:
		QWidget * ownerWidget;	/* kvuli chybovym zpravam */

		QString domainName;
		QTextStream outStream;
		QDomDocument currentDoc;
		QDomElement rootElement;
		QDomElement definitionElement;
		QDomElement operatorsElement;
		QDomElement problemsElement;

		bool basicStructureCheck(const QDomDocument & domainDocument);

		void writeLine(int indent, QString text);
		void writeItem(int indent, QString text, QString header = QString(), QString footer = QString());

		void writeAction(int indent, QDomElement diagram, QString name);
		
		void writeTask(QDomElement & diagram, const QString & name, const QString & targetDir);

		QString getDefinedTypes();
		QString getDefinedPredicates();
		QString getPredicateDefinition(QDomElement diagram, QString predName);
		QString getPredicateInstance(QDomElement predicate, QDomElement diagram, bool ques = true);
		QString getObjectList(QDomElement diagram);
		QString buildPredicateString(QString & name, QStringList * argNames,
							QStringList * argTypes = 0, bool ques = true);
		QString buildSetString(QStringList & positive, QStringList & negative);
		
		QStringList	generateArgumentNames(int size, char startingChar); 	
		QString getActionParameters(QDomElement diagram);
};

#endif
