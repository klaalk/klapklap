//
// Created by Klaus on 26/05/2019.
//

#include "textedit.h"
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



TextEdit::TextEdit(QWidget *parent)
    : QMainWindow(parent)
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
//    setWindowState(Qt::WindowMaximized);
    setWindowTitle(QCoreApplication::applicationName());

    this->setMouseTracking(true);
    textEdit = new QTextEdit(this);
    //Collega funzioni nostre a funzioni di QTextEdit
    connect(textEdit, &QTextEdit::currentCharFormatChanged, this, &TextEdit::onFormatChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
    connect(textEdit, &QTextEdit::textChanged, this, &TextEdit::onTextChange);

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
        helpMenu->addAction(tr("About"), this, &TextEdit::onAbout);
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

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TextEdit::onClipboardDataChanged);
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

bool TextEdit::load(const QString &f)
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

void TextEdit::loadCrdt(std::vector<std::list<KKCharPtr>> crdt)
{
    QTextCursor editorCurs = textEdit->textCursor();

    for(const auto& line : crdt) {
        for(const auto& charPtr : line) {
            applyRemoteChanges(CRDT_INSERT,
                               QString::fromStdString(charPtr->getSiteId()),
                               QChar::fromLatin1(charPtr->getValue()),
                               editorCurs.position()+1,
                               charPtr->getKKCharFont(),
                               charPtr->getKKCharColor()
                               );
        }
    }
}

void TextEdit::applyRemoteAlignmentChange(QString alignment)
{
    if (alignment=="left") {
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        actionAlignLeft->setChecked(true);
    }
    else if (alignment=="center") {
        textEdit->setAlignment(Qt::AlignHCenter);
        actionAlignCenter->setChecked(true);
    }
    else if (alignment=="right") {
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        actionAlignRight->setChecked(true);
    }
    else if (alignment=="justify") {
        textEdit->setAlignment(Qt::AlignJustify);
        actionAlignJustify->setChecked(true);
    }

    updateLabels();
}

void TextEdit::applyRemoteFormatChange(int position, QString font, QString color){
    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;
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

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::applyRemoteChanges(const QString& operation, const QString& name, const QString& text, int position, const QString& font, const QString& color) {
    //Blocco il cursore dell'editor.
    bool cursorBlocked = false;
    if (blockCursor)
        cursorBlocked = true;
    else blockCursor = true;


    //Faccio dei controlli sulla fattibilità dell'operazione.
    position = position > lastText.size() ? lastText.size() : position;
    position = position < 0 ? 0 : position;

    qDebug() << "[applyRemoteChanges]" << " value: " << text << " user: " << name <<  " position: " << position << " font: " << font << " color: " << color;
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

    //Prelevo il cursore remoto.
    KKCursor* remoteCurs = cursors.value(name);

    //Se non esiste quel cursore lo creo e lo memorizzo insieme alla label associata.
    if (remoteCurs == nullptr)
       createCursorAndLabel(remoteCurs, name, position);

    // Aggiorno il cursore remoto
    remoteCurs->setGlobalPositon(position);

    // Muovo il cursore dopo l'aggiornamento
    remoteCurs->moveLabels(textEdit->cursorRect(editorCurs));
    if(name != siteId)
        remoteCurs->showLabels();

    // Aggiorno e muovo tutti i cursori sulla base dell'operazione.
    qDebug() << "[applyRemoteChanges] - updateCursors" << " start: " << position << " length: " << text.size();

    updateCursors(position, operation == CRDT_INSERT ? text.size() : -text.size());

    lastText = textEdit->toPlainText();

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::applySiteIdsPositions(const QString& siteId, const QSharedPointer<QList<int>>& list){
    if(siteIdsPositions.contains(siteId))
        siteIdsPositions.remove(siteId);

    siteIdsPositions.insert(siteId, list);

    if (siteIdsClicked.contains(siteId))
        colorText(siteId);
    else clearColorText(siteId);
}

void TextEdit::applySiteIdClicked(const QString& siteId){
    if(siteIdsClicked.contains(siteId)) {
        clearColorText(siteId);
        siteIdsClicked.removeOne(siteId);
    }
    else {
        colorText(siteId);
    }
}

void TextEdit::setCurrentFileName(const QString &fileName)
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

void TextEdit::setChatDialog(ChatDialog *value)
{
    chatDialog = value;
    centralWidget()->layout()->addWidget(chatDialog);
}

void TextEdit::setMySiteId(QString mySiteId){
    siteId=std::move(mySiteId);
}

QString TextEdit::getMySiteId() {
    return this->siteId;
}

QTextEdit* TextEdit::getTextEdit(){
    return this->textEdit;
}

bool TextEdit::clickedOne(const QString& siteId) {
    return siteIdsClicked.contains(siteId);
}

bool TextEdit::clickedAny() {
    return !siteIdsClicked.isEmpty();
}


void TextEdit::fileNew()
{
    emit openFileDialog();
}

void TextEdit::closeEvent(QCloseEvent *e)
{
    e->ignore();
    hide();
    emit editorClosed();
}

void TextEdit::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event)
    updateLabels();
}

void TextEdit::fileOpen()
{
    emit openFileDialog();
}

bool TextEdit::fileSave()
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

bool TextEdit::fileSaveAs()
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

void TextEdit::filePrint()
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

void TextEdit::filePrintPreview()
{
#if QT_CONFIG(printpreviewdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &TextEdit::onPrintPreview);
    preview.exec();
#endif
}

void TextEdit::filePrintPdf()
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

void TextEdit::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormat(fmt);
}

void TextEdit::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormat(fmt);
}

void TextEdit::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormat(fmt);
}

void TextEdit::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormat(fmt);
}

void TextEdit::textSize(const QString &p)
{
    qreal pointSize = p.toDouble();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormat(fmt);
        updateLabels();
    }
}

void TextEdit::textStyle(int styleIndex)
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

void TextEdit::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormat(fmt);
    colorChanged(col);
}


void TextEdit::textAlign(QAction *a)
{
    QString alignment;
    if (a == actionAlignLeft){
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        alignment="left";
    }
    else if (a == actionAlignCenter){
        alignment="center";
        textEdit->setAlignment(Qt::AlignHCenter);
    }
    else if (a == actionAlignRight){
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        alignment="right";
    }
    else if (a == actionAlignJustify){
        alignment="justify";
        textEdit->setAlignment(Qt::AlignJustify);
    }
    updateLabels();
    emit(alignChange(alignment));
}

void TextEdit::onAbout()
{
    QMessageBox::about(this, tr("About"), tr("This example demonstrates Qt's "
                                             "rich text editing facilities in action, providing an example "
                                             "document for you to experiment with."));
}

void TextEdit::onPrintPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    textEdit->print(printer);
#endif
}


void TextEdit::onClipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::onFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
    updateLabels();
    //emit updateSiteIdsPositions(siteId);
}

void TextEdit::onCursorPositionChanged()
{
    // IMPORTANTE per le modifiche da remoto.
    if (blockCursor) return;

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


void TextEdit::onTextChange() {
    // IMPORTANTE per le modifiche da remoto.
    if(blockCursor) return;
    QString text = textEdit->toPlainText();

    dtl::Diff<char, string> d(lastText.toStdString(), text.toStdString());
    d.compose();

    int length = 0;
    long long position = INT_MAX;
    long long startDelIdx = -1, endDelIdx = -1;

    auto seq = d.getSes().getSequence();
    unsigned long start = 0;
    unsigned long end = static_cast<unsigned long>(seq.size()) -1;
    QTextCursor cursor = textEdit->textCursor();

    while (start < seq.size()) {
        auto elem = seq.at(start);
        if (elem.second.type == dtl::SES_ADD) {
            length++;
            position = position > elem.second.afterIdx-1 ? elem.second.afterIdx-1 : position;
            qDebug() << "[onTextChange] - (ADD) setPosition: " << elem.second.afterIdx-1;
            cursor.setPosition(static_cast<int>(elem.second.afterIdx-1));
            cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
            emit insertTextToCRDT(elem.first, static_cast<unsigned long>(elem.second.afterIdx-1), cursor.charFormat().font().toString(), cursor.charFormat().foreground().color().name());
        }
        if (elem.second.type == dtl::SES_COMMON) {
            qDebug() << "[onTextChange] - (COMMON) setPosition: " << elem.second.afterIdx-1;
            cursor.setPosition(static_cast<int>(elem.second.afterIdx-1));
            cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
            emit charFormatChange(static_cast<unsigned long>(elem.second.afterIdx-1), cursor.charFormat().font().toString(), cursor.charFormat().foreground().color().name());
        }
        elem = seq.at(end);
        if (elem.second.type == dtl::SES_DELETE) {
            length--;
            position = position > elem.second.beforeIdx-1 ? elem.second.beforeIdx-1 : position;
            if (endDelIdx == -1)
                endDelIdx = elem.second.beforeIdx;

            if (endDelIdx != -1)
                startDelIdx = elem.second.beforeIdx;
        }
        start++;
        end--;
        if ((elem.second.type != dtl::SES_DELETE || start >= seq.size()) && startDelIdx != -1 && endDelIdx != -1) {
            emit removeTextFromCRDT(static_cast<unsigned long>(startDelIdx-1), static_cast<unsigned long>(endDelIdx));
            startDelIdx = -1;
            endDelIdx = -1;
        }
    }
    qDebug() << "[onTextChange] - updateCursors" << " start: " << position << " length: " << length ;
    updateCursors(static_cast<int>(position), length);
    emit updateSiteIdsPositions(siteId);
    lastText = text;
}


void TextEdit::resetState() {
    blockCursor = false;
    fontSize=0;
    lastText = "";
    fileName = "";
    cursors.clear();
    siteIdsPositions.clear();
    textEdit->clear();
}


void TextEdit::setupFileActions()
{
    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/file.png"));
    QAction *a = menu->addAction(newIcon,  tr("&New"), this, &TextEdit::fileNew);
    tb->addAction(a);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/folder.png"));
    a = menu->addAction(openIcon, tr("&Open..."), this, &TextEdit::fileOpen);
    a->setShortcut(QKeySequence::Open);
    tb->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/save.png"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &TextEdit::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb->addAction(actionSave);
    a = menu->addAction(tr("Save &As..."), this, &TextEdit::fileSaveAs);
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/print.png"));
    a = menu->addAction(printIcon, tr("&Print..."), this, &TextEdit::filePrint);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    tb->addAction(a);

    const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/print.png"));
    menu->addAction(filePrintIcon, tr("Print Preview..."), this, &TextEdit::filePrintPreview);

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/export.png"));
    a = menu->addAction(exportPdfIcon, tr("&Export PDF..."), this, &TextEdit::filePrintPdf);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);

    menu->addSeparator();
#endif

    a = menu->addAction(tr("&Quit"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void TextEdit::setupEditActions()
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

void TextEdit::setupTextActions()
{
    QToolBar *tb = addToolBar(tr("Format Actions"));
    QMenu *menu = menuBar()->addMenu(tr("F&ormat"));

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/bold.png"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &TextEdit::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/italic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &TextEdit::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/underline.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Underline"), this, &TextEdit::textUnderline);
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
    connect(alignGroup, &QActionGroup::triggered, this, &TextEdit::textAlign);

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
    actionTextColor = menu->addAction(pix, tr("&Color..."), this, &TextEdit::textColor);
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

    connect(comboStyle, QOverload<int>::of(&QComboBox::activated), this, &TextEdit::textStyle);

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    foreach (int size, standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textSize);

}

void TextEdit::mergeFormat(const QTextCharFormat &format)
{
    bool cursorBlocked = false;
    if(blockCursor)
        cursorBlocked = true;
    else blockCursor = true;

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

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
    fontSize=f.pointSize();
}

void TextEdit::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEdit::alignmentChanged(Qt::Alignment a)
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

void TextEdit::colorText(const QString& siteId){
    if(!siteIdsPositions.contains(siteId))
        return;

    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

    //Se non ho ancora inserito il siteId nella mappa dei colori lo inserisco
    if(siteIdsColors.contains(siteId)) {
        QTextCursor cursor = textEdit->textCursor();
        QBrush color = siteIdsColors.value(siteId);
        for(int pos : *siteIdsPositions.value(siteId)) {
            qDebug() << "[colorText] - setPosition: " << pos;
            cursor.setPosition(pos);
            cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
            if (cursor.charFormat().background() != color){
                QTextCharFormat fmt = cursor.charFormat();
                fmt.setBackground(color);
                cursor.mergeCharFormat(fmt);
            }
        }
    }
    if(!siteIdsClicked.contains(siteId))
        siteIdsClicked.push_back(siteId);

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::clearColorText(const QString& siteId){
    if(!siteIdsPositions.contains(siteId))
        return;

    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

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
    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::updateCursors(int position, int value){
    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

    // Aggiorno e muovo tutti i cursori sulla base dell'operazione.
    QTextCursor editorCurs = textEdit->textCursor();
    for (KKCursor* c : cursors.values()) {
        if (c !=nullptr && c->getGlobalPositon() > position) {
            qDebug() << "[updateCursors] - setPosition: " << (c->getGlobalPositon() + value);
            c->setGlobalPositon(c->getGlobalPositon() + value);
            editorCurs.setPosition(c->getGlobalPositon());
            c->setLabelsSize(editorCurs.charFormat().font().pointSize());
            c->moveLabels(textEdit->cursorRect(editorCurs));
        }
    }

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::updateLabels() {
    bool cursorBlocked=false;
    if(blockCursor)
        cursorBlocked=true;
    else blockCursor=true;

    QTextCursor editorCurs = textEdit->textCursor();
    for(KKCursor* c : cursors.values()) {
        qDebug() << "[updateLabels] - setPosition: " << c->getGlobalPositon();
        editorCurs.setPosition(c->getGlobalPositon());
        c->setLabelsSize(editorCurs.charFormat().font().pointSize());
        c->moveLabels(textEdit->cursorRect(editorCurs));
    }

    // Sblocco il cursore dell'editor.
    if(!cursorBlocked)
        blockCursor=false;
}

void TextEdit::createCursorAndLabel(KKCursor*& remoteCurs, const QString& name, int postion) {
    //Creo il cursore
    remoteCurs = new KKCursor(postion);

    // Creo le label
    QLabel* qLbl = new QLabel(name, textEdit);
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

QBrush TextEdit::selectRandomColor(){
    QBrush color;
    do{
        int index=rand() % colors_.size();
        color= colors_.at(index);
    }
    while (siteIdsColors.values().contains(color));
    return color;
}
