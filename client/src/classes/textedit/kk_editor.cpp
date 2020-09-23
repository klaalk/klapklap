//
// Created by Klaus on 26/05/2019.
//

#include "kk_editor.h"
#include <QPlainTextEdit>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QLabel>
#include <QFontMetrics>
#include <QFont>
#include <QListWidgetItem>
#include <QTextCharFormat>


#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printer)
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#include <QPrinter>
#if QT_CONFIG(printpreviewdialog)
#include <QHBoxLayout>
#include <QPrintPreviewDialog>
#include <utility>
#endif
#endif
#endif


#ifdef Q_OS_MAC
const QString rsrcPath = ":/icon_set/png";
#else
const QString rsrcPath = ":/icon_set/png";
#endif




KKEditor::KKEditor(QWidget *parent)
    : QMainWindow(parent)
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
//    setWindowState(Qt::WindowMaximized);
    setWindowTitle(QCoreApplication::applicationName());

    this->setMouseTracking(true);
    textEdit = new KKTextEdit(this);
    //Collega funzioni nostre a funzioni di QTextEdit
    connect(textEdit, &QTextEdit::currentCharFormatChanged, this, &KKEditor::onFormatChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &KKEditor::onCursorPositionChanged);
    connect(textEdit, &KKTextEdit::textChangedEvent, this, &KKEditor::onTextChange);
    connect(textEdit, &KKTextEdit::wheelEventTriggered, this, &KKEditor::updateLabels);

    // Set layout
    QHBoxLayout *layout = new QHBoxLayout;
    textEdit->setProperty("class", "TextEdit");
    layout->addWidget(textEdit);
    //    layout->addWidget(myWidget2);

    // Set layout in QWidget
    QWidget *window = new QWidget();
    window->setLayout(layout);

    // Set QWidget as the central layout of the main window
    setProperty("class", "EditorCentral");
    setCentralWidget(window);
    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupTextActions();

    {
        QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
        helpMenu->addAction(tr("About"), this, &KKEditor::onAbout);
        helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    }

    QFont textFont("MS Shell Dlg 2");
    textFont.setStyleHint(QFont::System);
    textFont.setPointSize(10);
    textEdit->setFont(textFont);
    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    connect(textEdit->document(), &QTextDocument::modificationChanged,
            actionSave, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);


    setWindowModified(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isModified());
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

#ifndef QT_NO_CLIPBOARD
    actionCut->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    actionCopy->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &KKEditor::onClipboardDataChanged);
#endif

    textEdit->setFocus();
    setCurrentFileName(QString());

#ifdef Q_OS_MACOS
    // Use dark text on light background on macOS, also in dark mode.
    QPalette pal = textEdit->palette();
    pal.setColor(QPalette::Base, QColor(Qt::white));
    pal.setColor(QPalette::Text, QColor(Qt::black));
    textEdit->setPalette(pal);
#endif
}

bool KKEditor::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        textEdit->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        textEdit->setPlainText(str);
    }

    setCurrentFileName(f);
    return true;
}

void KKEditor::loadCrdt(std::vector<std::list<KKCharPtr>> crdt)
{
    QTextCursor editorCurs = textEdit->textCursor();

    for(const auto& line : crdt) {
        for(const auto& charPtr : line) {
            applyRemoteChanges(CRDT_INSERT,
                               QString::fromStdString(charPtr->getSiteId()),
                               QChar::fromLatin1(charPtr->getValue()),
                               editorCurs.position(),
                               charPtr->getKKCharFont(),
                               charPtr->getKKCharColor()
                               );
        }
    }
}
void KKEditor::alignmentRemoteChange(int alignment, unsigned long alignPos)
{
    int curStartPos;



    curStartPos = textEdit->textCursor().position();
    QTextCursor tmpCursor = textEdit->textCursor();
    tmpCursor.setPosition(static_cast<int>(alignPos));
    textEdit->setTextCursor(tmpCursor);

    QTextBlockFormat f;

    if (alignment==1) {
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        actionAlignLeft->setChecked(true);
    }
    else if (alignment==2) {
        textEdit->setAlignment(Qt::AlignHCenter);
        actionAlignCenter->setChecked(true);
    }
    else if (alignment==3) {
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        actionAlignRight->setChecked(true);
    }
    else if (alignment==4) {
        textEdit->setAlignment(Qt::AlignJustify);
        actionAlignJustify->setChecked(true);
    }

    tmpCursor.setPosition(curStartPos);
    textEdit->setTextCursor(tmpCursor);

    updateLabels();

}
//void KKEditor::applyRemoteAlignmentChange(QString alignment)
//{
//    if (alignment=="left") {
//        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
//        actionAlignLeft->setChecked(true);
//    }
//    else if (alignment=="center") {
//        textEdit->setAlignment(Qt::AlignHCenter);
//        actionAlignCenter->setChecked(true);
//    }
//    else if (alignment=="right") {
//        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
//        actionAlignRight->setChecked(true);
//    }
//    else if (alignment=="justify") {
//        textEdit->setAlignment(Qt::AlignJustify);
//        actionAlignJustify->setChecked(true);
//    }

//    updateLabels();
//}

void KKEditor::applyRemoteFormatChange(int position, QString font, QString color){
    qDebug() << "[applyRemoteFormatChange]" << " position: " << position << " font: " << font << " color: " << color;

    QTextCursor editorCurs = textEdit->textCursor();
    editorCurs.setPosition(position);
    editorCurs.movePosition(editorCurs.Right, QTextCursor::KeepAnchor);

    QFont fontNuovo;
    fontNuovo.fromString(font);
    QColor coloreNuovo(color);

    QTextCharFormat format = editorCurs.charFormat();
    if(format.font()!=fontNuovo)
        format.setFont(fontNuovo);

    if(format.foreground()!=coloreNuovo)
        format.setForeground(coloreNuovo);

    editorCurs.setCharFormat(format);
}

void KKEditor::applyRemoteChanges(const QString& operation, const QString& siteId, const QString& text, int position, const QString& font, const QString& color) {
    qDebug() << "[applyRemoteChanges]" << " value: " << text << " site id: " << siteId <<  " position: " << position << " font: " << font << " color: " << color;

    //Prelevo il cursore dell'editor.
    QTextCursor editorCurs = textEdit->textCursor();

    // Muovo il cursore dell'editor.
    editorCurs.setPosition(position);

    // Eseguo l'operazione.
    if(operation == CRDT_INSERT) {
       editorCurs.insertText(text);

       //Aggiorno formato
       applyRemoteFormatChange(position, font, color);

    } else if(operation == CRDT_DELETE) {

       editorCurs.deleteChar();
    }


    if(siteId != this->siteId) {
        //Prelevo il cursore remoto.
        KKCursor* remoteCurs = cursors.value(siteId);

        //Se non esiste quel cursore lo creo e lo memorizzo insieme alla label associata.
        if (remoteCurs == nullptr)
           createCursorAndLabel(remoteCurs, siteId, position);

        // Aggiorno il cursore remoto
        remoteCurs->setGlobalPositon(position);

        // Muovo il cursore dopo l'aggiornamento
        remoteCurs->moveLabels(textEdit->cursorRect(editorCurs));

        // Mostro le labels
        remoteCurs->showLabels();
    }

    // Aggiorno e muovo tutti i cursori sulla base dell'operazione.
    updateCursors(siteId, position, operation == CRDT_INSERT ? text.size() : -text.size());
}

void KKEditor::applySiteIdsPositions(const QString& siteId, const QSharedPointer<QList<int>>& list){
    if(siteIdsPositions.contains(siteId))
        siteIdsPositions.remove(siteId);

    siteIdsPositions.insert(siteId, list);

    if (siteIdsClicked.contains(siteId))
        colorText(siteId);
    else
        clearColorText(siteId);
}

void KKEditor::applySiteIdClicked(const QString& siteId){
    if(siteIdsClicked.contains(siteId)){
        clearColorText(siteId);
        siteIdsClicked.removeOne(siteId);
    }
    else
        colorText(siteId);
}

void KKEditor::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = fileName;
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(QString("%1 - %2").arg(shownName, siteId));
    setWindowModified(false);
}

void KKEditor::setParticipantAlias(QStringList participants)
{
    for(QString participant : participants) {
        QStringList params = participant.split(":");
        QString username = params.at(0);
        QString nick = params.at(1);
        participantsAlias.insert(username, nick);
    }
}

void KKEditor::addParticipant(const QString &username, const QString &nick)
{
    if (username.isEmpty() || nick.isEmpty())
        return;

    participantsAlias.insert(username, nick);
}

void KKEditor::removeParticipant(const QString &username)
{
    if (username.isEmpty())
        return;

    participantsAlias.remove(username);
}


void KKEditor::setChatDialog(ChatDialog *value)
{
    chatDialog = value;
    centralWidget()->layout()->addWidget(chatDialog);
}

void KKEditor::setMySiteId(QString mySiteId){
    siteId=std::move(mySiteId);
}

QString KKEditor::getMySiteId() {
    return this->siteId;
}

KKTextEdit* KKEditor::getTextEdit(){
    return this->textEdit;
}

bool KKEditor::clickedOne(const QString& siteId) {
    return siteIdsClicked.contains(siteId);
}

bool KKEditor::clickedAny() {
    return !siteIdsClicked.isEmpty();
}


void KKEditor::fileNew()
{
    emit openFileDialog();
}

void KKEditor::closeEvent(QCloseEvent *e)
{
    e->ignore();
    hide();
    emit editorClosed();
}

void KKEditor::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event)
    updateLabels();
}

void KKEditor::fileOpen()
{
    emit openFileDialog();
}

bool KKEditor::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();
    if (fileName.startsWith(QStringLiteral(":/")))
        return fileSaveAs();

    QTextDocumentWriter writer(fileName);
    bool success = writer.write(textEdit->document());
    if (success) {
        textEdit->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    } else {
        statusBar()->showMessage(tr("Could not write to file \"%1\"")
                                 .arg(QDir::toNativeSeparators(fileName)));
    }
    emit saveCRDTtoFile();
    return success;
}

bool KKEditor::fileSaveAs()
{
    QFileDialog fileDialog(this, tr("Save as..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes;
    mimeTypes << "application/vnd.oasis.opendocument.text" << "text/html" << "text/plain";
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.setDefaultSuffix("odt");
    if (fileDialog.exec() != QDialog::Accepted)
        return false;
    const QString fn = fileDialog.selectedFiles().first();
    setCurrentFileName(fn);
    return fileSave();
}

void KKEditor::filePrint()
{
#if QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);
    auto *dlg = new QPrintDialog(&printer, this);
    if (textEdit->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
        textEdit->print(&printer);
    delete dlg;
#endif
}

void KKEditor::filePrintPreview()
{
#if QT_CONFIG(printpreviewdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &KKEditor::onPrintPreview);
    preview.exec();
#endif
}

void KKEditor::filePrintPdf()
{
#ifndef QT_NO_PRINTER
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    textEdit->document()->print(&printer);
    statusBar()->showMessage(tr("Exported \"%1\"")
                             .arg(QDir::toNativeSeparators(fileName)));
#endif
}

void KKEditor::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormat(fmt);
}

void KKEditor::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormat(fmt);
}

void KKEditor::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormat(fmt);
}

void KKEditor::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormat(fmt);
}

void KKEditor::textSize(const QString &p)
{
    qreal pointSize = p.toDouble();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormat(fmt);
        updateLabels();
    }
}

void KKEditor::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();
    QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;

    switch (styleIndex) {
    case 1:
        style = QTextListFormat::ListDisc;
        break;
    case 2:
        style = QTextListFormat::ListCircle;
        break;
    case 3:
        style = QTextListFormat::ListSquare;
        break;
    case 4:
        style = QTextListFormat::ListDecimal;
        break;
    case 5:
        style = QTextListFormat::ListLowerAlpha;
        break;
    case 6:
        style = QTextListFormat::ListUpperAlpha;
        break;
    case 7:
        style = QTextListFormat::ListLowerRoman;
        break;
    case 8:
        style = QTextListFormat::ListUpperRoman;
        break;
    default:
        break;
    }

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    if (style == QTextListFormat::ListStyleUndefined) {
        blockFmt.setObjectIndex(-1);
        int headingLevel = styleIndex >= 9 ? styleIndex - 9 + 1 : 0; // H1 to H6, or Standard
        blockFmt.setHeadingLevel(headingLevel);
        cursor.setBlockFormat(blockFmt);

        int sizeAdjustment = headingLevel ? 4 - headingLevel : 0; // H1 to H6: +3 to -2
        QTextCharFormat fmt;
        fmt.setFontWeight(headingLevel ? QFont::Bold : QFont::Normal);
        fmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.mergeCharFormat(fmt);
        textEdit->mergeCurrentCharFormat(fmt);
    } else {
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        listFmt.setStyle(style);
        cursor.createList(listFmt);
    }

    cursor.endEditBlock();
}

void KKEditor::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormat(fmt);
    colorChanged(col);
}


void KKEditor::textAlign(QAction *a)
{
    int alignment;
    int alignStart;
    int alignEnd;

    if (a == actionAlignLeft){
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        alignment=1;
    }
    else if (a == actionAlignCenter){
        alignment=2;
        textEdit->setAlignment(Qt::AlignHCenter);
    }
    else if (a == actionAlignRight){
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        alignment=3;
    }else if (a == actionAlignJustify){
        alignment=4;
        textEdit->setAlignment(Qt::AlignJustify);
    }

    qDebug()<<"CURLINNUM:"<<textEdit->textCursor().block().blockNumber();
    qDebug()<<"CURCOLNUM:"<<textEdit->textCursor().columnNumber();


    if(textEdit->textCursor().hasSelection()){
        if(textEdit->textCursor().selectionEnd() > textEdit->textCursor().selectionStart()){
            alignStart=textEdit->textCursor().selectionStart();
            alignEnd=textEdit->textCursor().selectionEnd();
        }else{
            alignStart=textEdit->textCursor().selectionEnd();
            alignEnd=textEdit->textCursor().selectionStart();
        }

    }else{
        alignStart = textEdit->textCursor().position();
        alignEnd = alignStart;
    }

    updateLabels();
    emit(alignChange(alignment,alignStart,alignEnd));
}

void KKEditor::onAbout()
{
    QMessageBox::about(this, tr("About"), tr("This example demonstrates Qt's "
                                             "rich text editing facilities in action, providing an example "
                                             "document for you to experiment with."));
}

void KKEditor::onPrintPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    textEdit->print(printer);
#endif
}


void KKEditor::onClipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void KKEditor::onFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
//    updateLabels();
}

void KKEditor::onCursorPositionChanged()
{
    QTextList *list = textEdit->textCursor().currentList();

    if (list) {
        switch (list->format().style()) {
        case QTextListFormat::ListDisc:
            comboStyle->setCurrentIndex(1);
            break;
        case QTextListFormat::ListCircle:
            comboStyle->setCurrentIndex(2);
            break;
        case QTextListFormat::ListSquare:
            comboStyle->setCurrentIndex(3);
            break;
        case QTextListFormat::ListDecimal:
            comboStyle->setCurrentIndex(4);
            break;
        case QTextListFormat::ListLowerAlpha:
            comboStyle->setCurrentIndex(5);
            break;
        case QTextListFormat::ListUpperAlpha:
            comboStyle->setCurrentIndex(6);
            break;
        case QTextListFormat::ListLowerRoman:
            comboStyle->setCurrentIndex(7);
            break;
        case QTextListFormat::ListUpperRoman:
            comboStyle->setCurrentIndex(8);
            break;
        default:
            comboStyle->setCurrentIndex(-1);
            break;
        }
    } else {
        int headingLevel = textEdit->textCursor().blockFormat().headingLevel();
        comboStyle->setCurrentIndex(headingLevel ? headingLevel + 8 : 0);
    }
}


void KKEditor::onTextChange(QString operation, QString diff, int start, int end) {
    updateCursors(siteId, static_cast<int>(start), operation == INSERT ? diff.size() : -diff.size());

    if (operation == DELETE)
        emit removeTextFromCRDT(static_cast<unsigned long>(start), static_cast<unsigned long>(end));

    if (operation == INSERT) {
        QTextCursor cursor = textEdit->textCursor();
        for (int i = 0; i < diff.length(); i++) {
            cursor.setPosition(start + 1);
            cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
            emit insertTextToCRDT(diff.at(i).toLatin1(), static_cast<unsigned long>(start+i), cursor.charFormat().font().toString(), cursor.charFormat().foreground().color().name());
        }
    }

    emit updateSiteIdsPositions(siteId);
}


void KKEditor::resetState() {
    fontSize=0;
    fileName = "";
    cursors.clear();
    siteIdsPositions.clear();
    textEdit->clear();
}


void KKEditor::setupFileActions()
{
    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/file.png"));
    QAction *a = menu->addAction(newIcon,  tr("&New"), this, &KKEditor::fileNew);
    tb->addAction(a);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/folder.png"));
    a = menu->addAction(openIcon, tr("&Open..."), this, &KKEditor::fileOpen);
    a->setShortcut(QKeySequence::Open);
    tb->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/save.png"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &KKEditor::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb->addAction(actionSave);
    a = menu->addAction(tr("Save &As..."), this, &KKEditor::fileSaveAs);
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/print.png"));
    a = menu->addAction(printIcon, tr("&Print..."), this, &KKEditor::filePrint);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    tb->addAction(a);

    const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/print.png"));
    menu->addAction(filePrintIcon, tr("Print Preview..."), this, &KKEditor::filePrintPreview);

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/export.png"));
    a = menu->addAction(exportPdfIcon, tr("&Export PDF..."), this, &KKEditor::filePrintPdf);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);

    menu->addSeparator();
#endif

    a = menu->addAction(tr("&Quit"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void KKEditor::setupEditActions()
{
    QToolBar *tb = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/undo.png"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/redo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/scissors.png"));
    actionCut = menu->addAction(cutIcon, tr("Cu&t"), textEdit, &QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/copy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &QTextEdit::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/paste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Paste"), textEdit, &QTextEdit::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    tb->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void KKEditor::setupTextActions()
{
    QToolBar *tb = addToolBar(tr("Format Actions"));
    QMenu *menu = menuBar()->addMenu(tr("F&ormat"));

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/bold.png"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &KKEditor::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/italic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &KKEditor::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/underline.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Underline"), this, &KKEditor::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/left-align.png"));
    actionAlignLeft = new QAction(leftIcon, tr("&Left"), this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/center-align.png"));
    actionAlignCenter = new QAction(centerIcon, tr("C&enter"), this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/right-align.png"));
    actionAlignRight = new QAction(rightIcon, tr("&Right"), this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/justify.png"));
    actionAlignJustify = new QAction(fillIcon, tr("&Justify"), this);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    auto *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &KKEditor::textAlign);

    if (QApplication::isLeftToRight()) {
        alignGroup->addAction(actionAlignLeft);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignRight);
    } else {
        alignGroup->addAction(actionAlignRight);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignLeft);
    }
    alignGroup->addAction(actionAlignJustify);

    tb->addActions(alignGroup->actions());
    menu->addActions(alignGroup->actions());

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = menu->addAction(pix, tr("&Color..."), this, &KKEditor::textColor);
    tb->addAction(actionTextColor);

    tb = addToolBar(tr("Format Actions"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    comboStyle = new QComboBox(tb);
    tb->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    comboStyle->addItem("Ordered List (Roman lower)");
    comboStyle->addItem("Ordered List (Roman upper)");
    comboStyle->addItem("Heading 1");
    comboStyle->addItem("Heading 2");
    comboStyle->addItem("Heading 3");
    comboStyle->addItem("Heading 4");
    comboStyle->addItem("Heading 5");
    comboStyle->addItem("Heading 6");

    connect(comboStyle, QOverload<int>::of(&QComboBox::activated), this, &KKEditor::textStyle);

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &KKEditor::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    foreach (int size, standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &KKEditor::textSize);

}

void KKEditor::mergeFormat(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if(cursor.hasSelection()) {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        for(int i = start; i < end; i++) {
            qDebug() << "[mergeFormat] - setPosition: " << i;
            cursor.setPosition(i);
            cursor.movePosition(cursor.Right,QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(format);
            emit charFormatChange(static_cast<unsigned long>(i), cursor.charFormat().font().toString(), cursor.charFormat().foreground().color().name());
        }
    }
    textEdit->mergeCurrentCharFormat(format);
}

void KKEditor::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
    fontSize=f.pointSize();
}

void KKEditor::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void KKEditor::alignmentChanged(Qt::Alignment a)
{
    QString alignment;
    if (a & Qt::AlignLeft){
        actionAlignLeft->setChecked(true);
    }
    else if (a & Qt::AlignHCenter){
        actionAlignCenter->setChecked(true);
    }
    else if (a & Qt::AlignRight){
        actionAlignRight->setChecked(true);
    }
    else if (a & Qt::AlignJustify){
        actionAlignJustify->setChecked(true);
    }
}

void KKEditor::colorText(const QString& siteId){
    if(!siteIdsPositions.contains(siteId))
        return;

    //Se non ho ancora inserito il siteId nella mappa dei colori lo inserisco
    QBrush color;

    if(siteIdsColors.contains(siteId)) {
        color = siteIdsColors.value(siteId);
    } else if (siteId == this->siteId) {
        color = selectRandomColor();
        siteIdsColors.insert(this->siteId, color);
    }

    QTextCursor cursor = textEdit->textCursor();
    for(int pos : *siteIdsPositions.value(siteId)) {
        cursor.setPosition(pos);
        cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
        if (cursor.charFormat().background() != color){
            QTextCharFormat fmt = cursor.charFormat();
            fmt.setBackground(color);
            cursor.mergeCharFormat(fmt);
        }
    }
    if(!siteIdsClicked.contains(siteId))
        siteIdsClicked.push_back(siteId);
}

void KKEditor::clearColorText(const QString& siteId){
    if(!siteIdsPositions.contains(siteId))
        return;
    isColored = false;

    QTextCursor cursor = textEdit->textCursor();
    for(int pos : *siteIdsPositions.value(siteId)){
        qDebug() << "[clearColorText] - setPosition: " << pos;
        cursor.setPosition(pos);
        cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
        if (cursor.charFormat().background()!=Qt::white){
            QTextCharFormat fmt = cursor.charFormat();
            fmt.setBackground(Qt::white);
            cursor.mergeCharFormat(fmt);
        }
    }
}

void KKEditor::updateCursors(QString siteId, int position, int value){
    // Aggiorno e muovo tutti i cursori sulla base dell'operazione.
    QTextCursor editorCurs = textEdit->textCursor();
    for (KKCursor* c : cursors.values()) {
        if (c !=nullptr && c->getGlobalPositon() > position && c->getSiteId() != siteId) {
            int nuovaPos = c->getGlobalPositon() + value;
            nuovaPos = nuovaPos >= 0 ? nuovaPos : 0;
            qDebug() << "[updateCursors] - " << c->getLabelName() << " PREV " << c->getGlobalPositon() << " POS " << nuovaPos;
            c->setGlobalPositon(nuovaPos);
            editorCurs.setPosition(nuovaPos);
            c->setLabelsSize(editorCurs.charFormat().font().pointSize());
            c->moveLabels(textEdit->cursorRect(editorCurs));
        }
    }
}

void KKEditor::updateLabels() {
    QTextCursor editorCurs = textEdit->textCursor();
    for(KKCursor* c : cursors.values()) {
        qDebug() << "[updateLabels] - setPosition: " << c->getGlobalPositon();
        editorCurs.setPosition(c->getGlobalPositon());
        c->setLabelsSize(editorCurs.charFormat().font().pointSize());
        c->moveLabels(textEdit->cursorRect(editorCurs));
    }
}

void KKEditor::createCursorAndLabel(KKCursor*& remoteCurs, const QString& name, int postion) {
    //Creo il cursore
    remoteCurs = new KKCursor(name, postion);

    // Creo le label
    QLabel* qLbl = new QLabel(participantsAlias.value(name), textEdit);
    QLabel* qLbl2 = new QLabel("|", textEdit);

    //Seleziono randomicamente un colore dalla lista dei colori, controllo se era già stato usato.
    QBrush color = selectRandomColor();

    // Impost le labels.
    remoteCurs->setLabels(qLbl, qLbl2);
    remoteCurs->setLabelsStyle(color, fontSize);

    // Inserisco nella mappa dei colori.
    siteIdsColors.insert(name, color);
    cursors.insert(name, remoteCurs);
}

QBrush KKEditor::selectRandomColor(){
    QBrush color;
    do{
        int index=rand() % colors_.size();
        color= colors_.at(index);
    }
    while (siteIdsColors.values().contains(color));
    return color;
}
