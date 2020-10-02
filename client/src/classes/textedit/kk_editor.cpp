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
#include <QScrollBar>


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
    setMouseTracking(true);

    textEdit = new KKTextEdit(this);

    loaderGif = new QMovie(":/gif/animation.gif");
    loaderGif->setScaledSize(QSize(100, 100));
    loader = new QLabel();
    loader->setMovie(loaderGif);
    loader->setAlignment(Qt::AlignCenter);

    //Collega funzioni nostre a funzioni di QTextEdit
    connect(textEdit, &QTextEdit::currentCharFormatChanged, this, &KKEditor::onFormatChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &KKEditor::onCursorPositionChanged);
    connect(textEdit, &KKTextEdit::textChangedEvent, this, &KKEditor::onTextChange);
    connect(textEdit, &KKTextEdit::wheelEventTriggered, this, &KKEditor::updateLabels);
    connect(textEdit, &KKTextEdit::alignmentNotifyEvent, this, &KKEditor::notifyAlignment);

    // Set layout
    QHBoxLayout *editorLayout = new QHBoxLayout;
    textEdit->setProperty("class", "TextEdit");
    editorLayout->addWidget(textEdit);
    editorLayout->addWidget(loader);

    // Set layout in QWidget
    QWidget* editorView = new QWidget();
    editorView->setLayout(editorLayout);

    // Set QWidget as the central layout of the main window
    setProperty("class", "EditorCentral");
    setCentralWidget(editorView);
    textEdit->hide();
    loaderGif->start();
    loader->show();
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
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);

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

void KKEditor::clear()
{
    textEdit->clear();
}

void KKEditor::setLink(QString link_) {
    link = link_;
}

void KKEditor::loading(bool loading)
{
    if (loading) {
        textEdit->hide();
        loaderGif->start();
        loader->show();
    } else {
        loader->hide();
        loaderGif->stop();
        textEdit->show();
    }
}

void KKEditor::load(std::vector<std::list<KKCharPtr>> crdt, std::vector<int> alignments)
{
    clear();
    int startPos = 0;
    unsigned long lineIdx = 0;
    int editorCursorPos = 0;

    for(const auto& line : crdt) {
        if (lineIdx < alignments.size())
            applyRemoteAlignmentChange(alignments.at(lineIdx++), startPos);

        for(const auto& charPtr : line) {
            QString remoteSiteId = charPtr->getSiteId();
            applyRemoteTextChange(CRDT_INSERT,
                                  startPos,
                                  remoteSiteId,
                                  charPtr->getValue(),
                                  charPtr->getKKCharFont(),
                                  charPtr->getKKCharColor());

            if (siteId != remoteSiteId)
                applyRemoteCursorChange(remoteSiteId, startPos);

            if (siteId == remoteSiteId)
                editorCursorPos = startPos;

            startPos++;
        }
    }
    textEdit->document()->clearUndoRedoStacks();
    textEdit->setCursorPosition(editorCursorPos);
    updateCursors(siteId, -1, 1);
}
void KKEditor::applyRemoteAlignmentChange(int alignment, int alignPos)
{
//    qDebug() << QString("APPLY REMOTE [ALIGNM]: Tipo %1 in position %2").arg(QVariant(alignment).toString(), QVariant(alignPos).toString());
    textEdit->lockCursor();
    textEdit->setCursorPosition(alignPos);
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

    textEdit->unlockCursor();

    updateLabels();
}

void KKEditor::applyRemoteFormatChange(int position, QString siteId, QString font, QString color){
//    qDebug() << QString("APPLY REMOTE [FORMAT]: In position %1 con font %2 e color %3").arg(QVariant(position).toString(), font, color);

    //    textEdit->lockCursor();

    QTextCursor editorCurs = textEdit->cursorIn(position);
    editorCurs.movePosition(editorCurs.Right, QTextCursor::KeepAnchor);

    QFont fontNuovo;
    fontNuovo.fromString(font);
    QColor coloreNuovo(color);

    QTextCharFormat format = editorCurs.charFormat();

    if(format.font() != fontNuovo)
        format.setFont(fontNuovo);

    if(format.foreground() != coloreNuovo)
        format.setForeground(coloreNuovo);

    if (format.background() != siteIdsColors.value(siteId) && siteIdsClicked.contains(siteId))
        format.setBackground(siteIdsColors.value(siteId));
    else if (format.background() != Qt::white && !siteIdsClicked.contains(siteId))
        format.setBackground(Qt::white);

    editorCurs.mergeCharFormat(format);
    // Sblocco il cursore dell'editor.
    //    textEdit->unlockCursor();
}

void KKEditor::applyRemoteTextChange(const QString& operation, int position, const QString& siteId, const QChar& text, const QString& font, const QString& color) {
//    qDebug() << QString("APPLY REMOTE [%1]: %2 in %3 with font %4 and color %5").arg(operation, text, QVariant(position).toString(), font, color);

    // Eseguo l'operazione.
    if(operation == CRDT_INSERT) {
        //Prelevo il cursore dell'editor e inserisco il testo
        textEdit->lockCursor();
        QTextCursor editorCurs = textEdit->cursorIn(position);
        editorCurs.insertText(text);
        textEdit->unlockCursor();

        // Aggiorno formato
        applyRemoteFormatChange(position, siteId, font, color);
    } else if(operation == CRDT_DELETE) {
        //Prelevo il cursore dell'editor e inserisco il testo
        textEdit->lockCursor();
        QTextCursor editorCurs = textEdit->cursorIn(position);
        editorCurs.deleteChar();
        textEdit->unlockCursor();
    }

    // Sblocco il cursore dell'editor.

    int localCursorPosition = textEdit->cursorPosition();

    if (localCursorPosition > position) {
        int newLocalCursorPosition = operation == CRDT_INSERT ?
                    localCursorPosition + 1 : localCursorPosition-1;
        textEdit->setCursorPosition(newLocalCursorPosition);
    }

    textEdit->document()->clearUndoRedoStacks();
}

void KKEditor::applyRemoteCursorChange(const QString &siteId, int position)
{
    KKCursor* remote = cursors.value(siteId);
    if (remote == nullptr)
        createCursorAndLabel(remote, siteId, position);
    remote->setGlobalPositon(position);
    remote->showLabels();
}

void KKEditor::applySiteIdsPositions(const QString& siteId, const QSharedPointer<QList<int>>& list){
    if(siteIdsPositions.contains(siteId))
        siteIdsPositions.remove(siteId);

    siteIdsPositions.insert(siteId, list);
}

QBrush KKEditor::applySiteIdClicked(const QString& siteId){
    QBrush color = Qt::white;

    if (siteIdsClicked.contains(siteId)) {
        siteIdsClicked.removeOne(siteId);
    } else {
        if(siteId != getMySiteId())
            textEdit->setCursorPosition(cursors.value(siteId)->getGlobalPositon());
        else
            textEdit->restoreCursorPosition();

        if (siteIdsColors.contains(siteId))
            color = siteIdsColors.value(siteId);
        else if (siteId == this->siteId) {
            color = selectRandomColor();
            siteIdsColors.insert(this->siteId, color);
        }

        if(!siteIdsClicked.contains(siteId))
            siteIdsClicked.push_back(siteId);
    }

    colorText(siteId, color);
    return color;
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


void KKEditor::setChatDialog(KKChat *value)
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
    QString link_ = (link != nullptr && !link.isEmpty()) ? link : "";
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(link_);
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
    emit saveCrdtTtoFile();
    return true;
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
    int alignment=0;
    int alignStart;
    int alignEnd;

    if (a == actionAlignLeft){
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        alignment=1;
    }
    else if (a == actionAlignCenter){
        textEdit->setAlignment(Qt::AlignHCenter);
        alignment=2;
    }
    else if (a == actionAlignRight){
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        alignment=3;
    }
    else if (a == actionAlignJustify){
        textEdit->setAlignment(Qt::AlignJustify);
        alignment=4;
    }

    if (textEdit->textCursor().hasSelection()) {
        if (textEdit->textCursor().selectionEnd() > textEdit->textCursor().selectionStart()){
            alignStart=textEdit->textCursor().selectionStart();
            alignEnd=textEdit->textCursor().selectionEnd();
        } else {
            alignStart=textEdit->textCursor().selectionEnd();
            alignEnd=textEdit->textCursor().selectionStart();
        }

    } else {
        alignStart = textEdit->textCursor().position();
        alignEnd = alignStart;
    }

    emit(alignChange(alignment, alignStart, alignEnd));
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
    //updateLabels();
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

    updateCursors(siteId, static_cast<int>(start), operation == INS ? diff.size() : -diff.size());
    updateLabels();

    if (operation == DEL)
        emit removeTextFromCrdt(static_cast<unsigned long>(start), static_cast<unsigned long>(end), diff);

    if (operation == INS) {
        QTextCursor cursor = textEdit->textCursor();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        QTextCharFormat format = cursor.charFormat();
        if (format.background() != siteIdsColors.value(siteId) && siteIdsClicked.contains(siteId))
            format.setBackground(siteIdsColors.value(siteId));
        else if (format.background() != Qt::white && !siteIdsClicked.contains(siteId))
            format.setBackground(Qt::white);
        cursor.mergeCharFormat(format);

        QStringList fonts, colors;
        QList<QChar> values;
        int i = start;
        for (QChar value : diff) {
            cursor.setPosition(i++);
            cursor.movePosition(cursor.Right, QTextCursor::KeepAnchor);
            values.push_back(value);
            QTextCharFormat format = cursor.charFormat();
            fonts.push_back(format.font().toString());
            colors.push_back(format.foreground().color().name());
        }

        emit insertTextToCrdt(static_cast<unsigned long>(start), values, fonts, colors);
    }

    emit updateSiteIdsPositions(siteId);
}

int KKEditor::getCurrentAlignment(int pos){
    QTextCursor cursor = textEdit->cursorIn(pos);
    Qt::Alignment a = cursor.blockFormat().alignment();
    int alignment = 0;
    if (a == (Qt::AlignLeft|Qt::AlignLeading) || a == (Qt::AlignLeading|Qt::AlignAbsolute)){
        alignment=1;
    }
    else if (a == Qt::AlignHCenter){
        alignment=2;
    }
    else if (a==(Qt::AlignTrailing|Qt::AlignAbsolute)){
        alignment=3;
    }
    else if (a == Qt::AlignJustify){
        alignment=4;
    }
    return alignment;
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

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/link.png"));
    QAction *a = menu->addAction(newIcon,  tr("&Share"), this, &KKEditor::fileNew);
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
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &KKTextEdit::textUndo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/redo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &KKTextEdit::textRedo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/scissors.png"));
    actionCut = menu->addAction(cutIcon, tr("Cu&t"), textEdit, &KKTextEdit::textCut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/copy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &KKTextEdit::textCopy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/paste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Paste"), textEdit, &KKTextEdit::textPaste);
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

        cursor.setPosition(start);
        cursor.mergeCharFormat(format);

        QString font = cursor.charFormat().font().toString();
        QString color = cursor.charFormat().foreground().color().name();
        emit charFormatChange(static_cast<unsigned long>(start), static_cast<unsigned long>(end), font, color);
    }
    textEdit->mergeCurrentCharFormat(format);
    updateLabels();
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

void KKEditor::colorText(const QString& siteId, QBrush color) {
    if(!siteIdsPositions.contains(siteId))
        return;

    //    textEdit->lockCursor();

    //Se non ho ancora inserito il siteId nella mappa dei colori lo inserisco
    QTextCursor cursor = textEdit->textCursor();
    int prevPos = -1;
    int start = 0;
    int end = 0;
    int last = siteIdsPositions.value(siteId)->last();

    for(int pos : *siteIdsPositions.value(siteId)) {

        if (prevPos == -1) {
            start = pos;
            prevPos = pos;
            end = pos;
        }

        if (pos-prevPos > 1 || pos == last) {
            end = prevPos;
            cursor.setPosition(start, QTextCursor::MoveAnchor);
            cursor.setPosition(end+1, QTextCursor::KeepAnchor);
            qDebug() << "[color da " << start << " a " << end+1;
            QTextCharFormat fmt;
            fmt.setBackground(color);
            cursor.mergeCharFormat(fmt);
            start = pos;
        }
        prevPos = pos;
    }
    //    // Sblocco il cursore dell'editor.
    //    textEdit->unlockCursor();
    qDebug() << "[color text]";
}

void KKEditor::updateCursors(QString siteId, int position, int value){
    // Aggiorno e muovo tutti i cursori sulla base dell'operazione.
    QTextCursor editorCurs = textEdit->textCursor();
    for (KKCursor* c : cursors.values()) {
        if (c !=nullptr) {
            int nuovaPos = c->getGlobalPositon();

            if (c->getGlobalPositon() > position && c->getSiteId() != siteId)
                nuovaPos += value;

            nuovaPos = nuovaPos >= 0 ? nuovaPos : 0;

            if (c->getGlobalPositon() > position || siteId == c->getSiteId()) {
                c->setGlobalPositon(nuovaPos);
                editorCurs.setPosition(nuovaPos);
                c->setLabelsSize(editorCurs.charFormat().font().pointSize());
                c->moveLabels(textEdit->cursorRect(editorCurs));
            }
        }
    }
}

void KKEditor::updateLabels() {
    QTextCursor editorCurs = textEdit->textCursor();
    int fontMax;
    for(KKCursor* c : cursors.values()) {
        editorCurs.setPosition(c->getGlobalPositon());
        int fontSx=editorCurs.charFormat().font().pointSize();
        fontMax=fontSx;
        if(editorCurs.position()<textEdit->document()->toPlainText().length()){
            if(textEdit->document()->toPlainText().at(editorCurs.position())!="\xa"){
                editorCurs.movePosition(editorCurs.Right, QTextCursor::KeepAnchor);
                int fontDx=editorCurs.charFormat().font().pointSize();
                if(fontDx>fontSx)
                    fontMax=fontDx;
            }
            editorCurs.setPosition(c->getGlobalPositon());
        }
        c->setLabelsSize(fontMax);
        c->moveLabels(textEdit->cursorRect(editorCurs));
    }
}

void KKEditor::createCursorAndLabel(KKCursor*& remoteCurs, const QString& siteId, int position) {
    //Creo il cursore
    remoteCurs = new KKCursor(siteId, position);

    // Creo le label
    QLabel* qLbl = new QLabel(participantsAlias.value(siteId), textEdit);
    QLabel* qLbl2 = new QLabel("│", textEdit);

    //Seleziono randomicamente un colore dalla lista dei colori, controllo se era già stato usato.
    QBrush color = selectRandomColor();

    // Inserisco nella mappa dei colori.
    siteIdsColors.insert(siteId, color);
    cursors.insert(siteId, remoteCurs);

    // Impost le labels.
    remoteCurs->setLabels(qLbl, qLbl2);
    remoteCurs->setLabelsStyle(color.color().toRgb(), fontSize);
}

QBrush KKEditor::selectRandomColor(){
    QBrush color;
    int index;

    if (primaryColors_.size()!=0) {
        index=rand() % primaryColors_.size();
        return primaryColors_.takeAt(index);
    }

    index=rand() % secondaryColors_.size();

    return secondaryColors_.takeAt(index);
}
