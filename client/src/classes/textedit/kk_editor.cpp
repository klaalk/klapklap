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
    setWindowTitle(QCoreApplication::applicationName());
    setMouseTracking(true);

    textEdit = new KKTextEdit(this);

    textEdit->setAcceptDrops(false);
    setAcceptDrops(false);

    textEdit->setContextMenuPolicy(Qt::CustomContextMenu);

    loaderGif = new QMovie(":/gif/animation.gif");
    loaderGif->setScaledSize(QSize(100, 100));
    loader = new QLabel();
    loader->setMovie(loaderGif);
    loader->setAlignment(Qt::AlignCenter);

    //Collega funzioni nostre a funzioni di QTextEdit
    connect(textEdit, &QTextEdit::currentCharFormatChanged, this, &KKEditor::onFormatChanged);
    connect(textEdit, &KKTextEdit::textChangedEvent, this, &KKEditor::onTextChange);
    connect(textEdit, &KKTextEdit::wheelEventTriggered, this, &KKEditor::updateLabels);
    connect(textEdit, &KKTextEdit::alignmentNotifyEvent, this, &KKEditor::notifyAlignment);
    connect(textEdit, &QTextEdit::customContextMenuRequested, this, &KKEditor::showContextMenu);
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
        helpMenu->addAction(tr("Print CRDT"), this, &KKEditor::printCrdt);

    }

    QFont textFont("Arial");
    textFont.setStyleHint(QFont::System);
    textFont.setPointSize(10);
    textEdit->setFont(textFont);
    textEdit->setStyleSheet("QTextEdit{ padding-top: 15; }");

    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    connect(textEdit->document(), &QTextDocument::modificationChanged,
            actionSave, &QAction::setEnabled);
    connect(textEdit, &QTextEdit::undoAvailable, actionUndo, &QAction::setEnabled);
    connect(textEdit, &QTextEdit::redoAvailable, actionRedo, &QAction::setEnabled);

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
        setEnabled(false);
    } else {
        setEnabled(true);
        loader->hide();
        loaderGif->stop();
        textEdit->show();
    }
}

void KKEditor::load(std::vector<std::list<KKCharPtr>> crdt, std::vector<int> alignments)
{
    clear();
    QMap<QString, int> remotePositions;

    int remotePosition = 0;
    int localPostion = 0;
    unsigned long lineIdx = 0;

    for(const auto& line : crdt) {
        if (lineIdx < alignments.size())
            applyRemoteAlignmentChange(alignments.at(lineIdx++), remotePosition);

        for(const auto& charPtr : line) {
            QString remoteSiteId = charPtr->getSiteId();
            applyRemoteTextChange(CRDT_INSERT,
                                  remotePosition,
                                  remoteSiteId,
                                  charPtr->getValue(),
                                  charPtr->getKKCharFont(),
                                  charPtr->getKKCharColor());

            if (siteId == remoteSiteId)
                localPostion = remotePosition;
            else
                remotePositions.insert(remoteSiteId, remotePosition);

            remotePosition++;
        }
    }
    textEdit->setLocalCursorPosition(localPostion);
    textEdit->document()->setModified(false);

    for (auto entry : remotePositions.toStdMap())
        applyRemoteCursorChange(entry.first, entry.second);
}

void KKEditor::applyRemoteAlignmentChange(int alignment, int alignPos)
{
    textEdit->setCursorPosition(alignPos);
    QTextBlockFormat f;
    if (alignment == 1) {
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        actionAlignLeft->setChecked(true);
    }
    else if (alignment == 2) {
        textEdit->setAlignment(Qt::AlignHCenter);
        actionAlignCenter->setChecked(true);
    }
    else if (alignment == 3) {
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        actionAlignRight->setChecked(true);
    }
    else if (alignment == 4) {
        textEdit->setAlignment(Qt::AlignJustify);
        actionAlignJustify->setChecked(true);
    }
}

void KKEditor::applyRemoteFormatChange(int position, QString siteId, QString font, QString color, const QString& operation){
    // Recupero il cursore dell'editor
    QTextCursor editorCurs = textEdit->getCursor(position);
    editorCurs.movePosition(editorCurs.Right, QTextCursor::KeepAnchor);

    // Creo il nuovo font
    QFont fontNuovo;
    fontNuovo.fromString(font);

    // Creo il nuovo colore
    QColor coloreNuovo(color);

    // Recupero il formato presente e lo modifico con il nuovo stile
    QTextCharFormat format = editorCurs.charFormat();
    if(format.font() != fontNuovo)
        format.setFont(fontNuovo);
    if(format.foreground() != coloreNuovo)
        format.setForeground(coloreNuovo);

    // Controllo che il background sia coerente
    if (operation == CRDT_INSERT){
    if (format.background() != siteIdsColors.value(siteId) && siteIdsClicked.contains(siteId))
        format.setBackground(siteIdsColors.value(siteId));
    else if (format.background() != Qt::white && !siteIdsClicked.contains(siteId))
        format.setBackground(Qt::white);
    }

    // Mergio le modifiche
    editorCurs.mergeCharFormat(format);
}

void KKEditor::applyRemoteTextChange(const QString& operation, int position, const QString& siteId, const QChar& text, const QString& font, const QString& color) {
    // Eseguo l'operazione.
    QTextCursor editorCurs = textEdit->getCursor(position);

    if (operation == CRDT_INSERT) {
        //Prelevo il cursore dell'editor e inserisco il testo
        editorCurs.insertText(text);

    } else if (operation == CRDT_DELETE) {
        //Prelevo il cursore dell'editor e inserisco il testo
        editorCurs.deleteChar();
    }

    if (operation == CRDT_FORMAT || operation == CRDT_INSERT) {
        // Aggiorno formato
        applyRemoteFormatChange(position, siteId, font, color, operation);
    }
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
        if(siteId != getMySiteId() && cursors.contains(siteId))
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

    setColorText(siteId, color);
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
    for (QString participant : participants) {
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

void KKEditor::clearUndoRedoStack()
{
    textEdit->document()->clearUndoRedoStacks();
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

void KKEditor::closeEvent(QCloseEvent *event)
{
    event->ignore();
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
    textEdit->document()->setModified(false);
    textEdit->document()->clearUndoRedoStacks();
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
    int alignment = 0;
    int alignStart;
    int alignEnd;

    if (a == actionAlignLeft && textEdit->alignment() != (Qt::AlignLeft | Qt::AlignAbsolute)){
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        alignment = 1;
    }
    else if (a == actionAlignCenter && textEdit->alignment() != (Qt::AlignHCenter)){
        textEdit->setAlignment(Qt::AlignHCenter);
        alignment = 2;
    }
    else if (a == actionAlignRight && textEdit->alignment() != (Qt::AlignRight | Qt::AlignAbsolute)){
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        alignment = 3;
    }
    else if (a == actionAlignJustify && textEdit->alignment() != (Qt::AlignJustify) ){
        textEdit->setAlignment(Qt::AlignJustify);
        alignment = 4;
    }

    if (alignment >= 1 && alignment <= 4) {
        if (textEdit->textCursor().hasSelection()) {
            if (textEdit->textCursor().selectionEnd() > textEdit->textCursor().selectionStart()){
                alignStart = textEdit->textCursor().selectionStart();
                alignEnd = textEdit->textCursor().selectionEnd();
            } else {
                alignStart = textEdit->textCursor().selectionEnd();
                alignEnd = textEdit->textCursor().selectionStart();
            }

        } else {
            alignStart = textEdit->textCursor().position();
            alignEnd = alignStart;
        }

        emit(alignChange(alignment, alignStart, alignEnd));
    }
}

void KKEditor::onAbout()
{
    QMessageBox::about(this, tr("About"), tr("Klap Klap ti permette di condividere il tuo documento in tempo reale con gli altri utenti tramite un link di condivisione che potrai copiare selezionando l'opzione 'Share'.\nAll'interno dell'editor puoi comunicare con gli attuali partecipanti attraverso la nostra chat integrata.\nRicorda! Lo storico dei messaggi verrà cancellato non appena non ci saranno più partecipanti attivi."));
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
}

void KKEditor::onTextChange(QString operation, QString diff, int start, int end) {

    updateCursors(siteId, static_cast<int>(start), operation == CRDT_INSERT ? diff.size() : -diff.size());
    updateLabels();

    if (operation == CRDT_DELETE)
        emit removeTextFromCrdt(static_cast<unsigned long>(start), static_cast<unsigned long>(end), diff);

    if (operation == CRDT_INSERT) {
        updateColorText(start, end, siteId);
        QTextCursor cursor = textEdit->textCursor();
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
}

void KKEditor::showContextMenu(const QPoint &pos)
{
    QMenu menu(tr("Context menu"), this);
    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/undo.png"));
    QAction* undoAction = menu.addAction(undoIcon, tr("&Undo"), textEdit, &KKTextEdit::textUndo);
    undoAction->setEnabled(textEdit->getIsUndoAvailable());

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/redo.png"));
    QAction* redoAction = menu.addAction(redoIcon, tr("&Redo"), textEdit, &KKTextEdit::textRedo);
    redoAction->setEnabled(textEdit->getIsRedoAvailable());

    menu.addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/copy.png"));
    QAction* copyAction = menu.addAction(copyIcon, tr("&Copia"), textEdit, &KKTextEdit::textCopy);
    copyAction->setEnabled(textEdit->textCursor().hasSelection());

    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/scissors.png"));
    QAction* cutAction = menu.addAction(cutIcon, tr("&Taglia"), textEdit, &KKTextEdit::textCut);
    cutAction->setEnabled(textEdit->textCursor().hasSelection());

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/paste.png"));
    QAction* pasteAction = menu.addAction(pasteIcon, tr("&Incolla"), textEdit, &KKTextEdit::textPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        pasteAction->setEnabled(md->hasText());
#endif
    menu.exec(mapToGlobal(pos));
}

bool KKEditor::isLoading()
{
    return loader->isVisible();
}

int KKEditor::getCurrentAlignment(int pos){
    QTextCursor cursor = textEdit->getCursor(pos);
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
    QAction *a = menu->addAction(newIcon,  tr("&Condividi"), this, &KKEditor::fileNew);
    tb->addAction(a);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/folder.png"));
    a = menu->addAction(openIcon, tr("&Apri..."), this, &KKEditor::fileOpen);
    a->setShortcut(QKeySequence::Open);
    tb->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/save.png"));
    actionSave = menu->addAction(saveIcon, tr("&Salva"), this, &KKEditor::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb->addAction(actionSave);
    a = menu->addAction(tr("Salva &come..."), this, &KKEditor::fileSaveAs);
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/print.png"));
    a = menu->addAction(printIcon, tr("&Stampa..."), this, &KKEditor::filePrint);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    tb->addAction(a);

    const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/print.png"));
    menu->addAction(filePrintIcon, tr("Anteprima di stampa..."), this, &KKEditor::filePrintPreview);

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/export.png"));
    a = menu->addAction(exportPdfIcon, tr("&Esporta come PDF..."), this, &KKEditor::filePrintPdf);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);

    menu->addSeparator();
#endif

    a = menu->addAction(tr("&Chiudi"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void KKEditor::setupEditActions()
{
    QToolBar *tb = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Modifica"));

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
    actionCut = menu->addAction(cutIcon, tr("Taglia"), textEdit, &KKTextEdit::textCut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/copy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copia"), textEdit, &KKTextEdit::textCopy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/paste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Incolla"), textEdit, &KKTextEdit::textPaste);
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
    actionTextBold = menu->addAction(boldIcon, tr("&Grassetto"), this, &KKEditor::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/italic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Corsivo"), this, &KKEditor::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/underline.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Sottolineatura"), this, &KKEditor::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/left-align.png"));
    actionAlignLeft = new QAction(leftIcon, tr("&Sinistra"), this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/center-align.png"));
    actionAlignCenter = new QAction(centerIcon, tr("C&entro"), this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/right-align.png"));
    actionAlignRight = new QAction(rightIcon, tr("&Destra"), this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/justify.png"));
    actionAlignJustify = new QAction(fillIcon, tr("&Giustifica"), this);
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
    actionTextColor = menu->addAction(pix, tr("&Colore..."), this, &KKEditor::textColor);
    tb->addAction(actionTextColor);

    tb = addToolBar(tr("Format Actions"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    QFontDatabase Datab;
    QStringList standardFonts = Datab.families(QFontDatabase::Any);


    comboFont = new QFontComboBox(tb);
    removeNonCompatibleFonts(comboFont);


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

void KKEditor::setColorText(const QString& siteId, QBrush color) {

    if (!siteIdsPositions.contains(siteId)
            || siteIdsPositions.value(siteId)->isEmpty())

        return;

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
            if (pos == last && pos-prevPos==1)
                end = pos;

            cursor.setPosition(start, QTextCursor::MoveAnchor);
            cursor.setPosition(end+1, QTextCursor::KeepAnchor);

            QTextCharFormat fmt;
            fmt.setBackground(color);
            cursor.mergeCharFormat(fmt);
            start = pos;

            if (pos == last && pos-prevPos>1){
                cursor.setPosition(pos, QTextCursor::MoveAnchor);
                cursor.setPosition(pos+1, QTextCursor::KeepAnchor);

                QTextCharFormat fmt;
                fmt.setBackground(color);
                cursor.mergeCharFormat(fmt);
            }

        }
        prevPos = pos;
    }

    textEdit->document()->clearUndoRedoStacks();
}

void KKEditor::updateColorText(int start, int end, const QString& siteId)
{
    QTextCursor cursor = textEdit->textCursor();
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);

    QTextCharFormat format = cursor.charFormat();
    if (format.background() != siteIdsColors.value(siteId) && siteIdsClicked.contains(siteId)) {
        format.setBackground(siteIdsColors.value(siteId));
        cursor.mergeCharFormat(format);
    } else if (format.background() != Qt::white && !siteIdsClicked.contains(siteId)) {
        format.setBackground(Qt::white);
        cursor.mergeCharFormat(format);
    }
}

/// Aggiorno i cursori remoti sulla base della posizione iniziale e numero di operazioni.
void KKEditor::updateCursors(QString siteId, int startPosition, int operations){
    int maxPosition = textEdit->toPlainText().length();
    for (KKCursor* c : cursors.values()) {
        if (c != nullptr) {
            int cursorPosition = c->getGlobalPositon();
            if (cursorPosition > startPosition) {

                if (c->getSiteId() != siteId)
                    cursorPosition += operations;

                cursorPosition = cursorPosition >= 0 ? cursorPosition : 0;
                cursorPosition = cursorPosition <= maxPosition ? cursorPosition : maxPosition;
                c->setGlobalPositon(cursorPosition);
                KKLogger::log(QString("[updateCursors] - %1 in posizione %2").arg(c->getSiteId(), QString::number(cursorPosition)), "EDITOR");
            }
        }
    }
}

void KKEditor::updateLocalCursor(int startPosition, int delta)
{
    // Controllo la posizione dell'editor locale
    // e ne eseguo l'aggiornamento se necessario
    int position = textEdit->getLocalCursorPosition();
    if (position > startPosition) {
        position += delta;
        textEdit->setLocalCursorPosition(position);
    } else {
        textEdit->restoreCursorPosition();
    }
}

// Muovo tutte le labels sulla base della posizione salvata
void KKEditor::updateLabels() {
    QTextCursor editorCurs = textEdit->textCursor();
    QString text = textEdit->toPlainText();
    int fontMax=-1;
    int positionMax = text.length();
    for(KKCursor* c : cursors.values()) {
        editorCurs.setPosition(c->getGlobalPositon());
        int fontSx = editorCurs.charFormat().font().pointSize();
        int fontDx = -1;

        fontMax = fontSx;
        if(editorCurs.position() < positionMax) {
            if (text.at(editorCurs.position()) != "\xa"){
                editorCurs.movePosition(editorCurs.Right, QTextCursor::KeepAnchor);
                fontDx = editorCurs.charFormat().font().pointSize();
                if(fontDx > fontSx)
                    fontMax = fontDx;
            }
            editorCurs.setPosition(c->getGlobalPositon());
        }
        c->setLabelsSize(fontMax);
        QRect rect;
        rect.setX(textEdit->cursorRect(editorCurs).x());
        int yH = textEdit->cursorRect(editorCurs).y() + PADDING;
        if(textEdit->cursorRect(editorCurs).height()>QFontMetrics(editorCurs.charFormat().font()).height() && ((fontDx!=-1 && fontSx==fontDx) || editorCurs.position()==textEdit->document()->toPlainText().length())) {
            yH= yH + textEdit->cursorRect(editorCurs).height() - QFontMetrics(editorCurs.charFormat().font()).height() - static_cast<int>(textEdit->cursorRect(editorCurs).height() / 7.5);
        }
        rect.setY(yH);
        c->moveLabels(rect);
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
    QColor tmp = color.color();
    tmp.setAlpha(250);
    remoteCurs->setLabelsStyle(tmp, fontSize);
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

void KKEditor::removeNonCompatibleFonts(QComboBox *comboFont){
    QFontDatabase Datab;
    QStringList standardFonts = Datab.families(QFontDatabase::Any);
    QStringList compatibili = getCompatibleFonts();
    bool flag=false;


       for(int i=0;i< standardFonts.size();i++){
            if(Datab.isPrivateFamily(standardFonts[i])){
                standardFonts.removeAt(i);
                i--;
            }

        }


       for(int j=0; j<standardFonts.size();j++){

           for(int i=0;i<compatibili.size();i++){
               if(compatibili[i]==standardFonts[j]){
               flag=true;
               }
           }
             if(!flag){

                 comboFont->removeItem(j);
                 standardFonts.removeAt(j);
                 j--;
             }
             flag=false;



       }

}


QStringList KKEditor::getCompatibleFonts(){
    return fontCompatibili;
}
