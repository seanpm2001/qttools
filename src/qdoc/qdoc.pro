
!force_bootstrap {
    requires(qtConfig(xmlstreamwriter))
}

option(host_build)
QT = core
qtHaveModule(qmldevtools-private) {
    QT += qmldevtools-private
} else {
    DEFINES += QT_NO_DECLARATIVE
}

LIBS += $$CLANG_LIBS
INCLUDEPATH += $$CLANG_INCLUDEPATH
!disable_external_rpath: QMAKE_RPATHDIR += $$CLANG_LIBDIR
DEFINES += $$shell_quote(CLANG_RESOURCE_DIR=\"$${CLANG_LIBDIR}/clang/$${CLANG_VERSION}/include\")

DEFINES += \
    QDOC2_COMPAT

INCLUDEPATH += $$QT_SOURCE_TREE/src/tools/qdoc \
               $$QT_SOURCE_TREE/src/tools/qdoc/qmlparser

# Increase the stack size on MSVC to 4M to avoid a stack overflow
win32-icc*|win32-msvc*:{
    QMAKE_LFLAGS += /STACK:4194304
} else {
    # (MSVC objects to defining a function in the locale.h dllimport context)
    DEFINES += HEINOUS_SYSINC_HACK
}

HEADERS += atom.h \
           clangcodeparser.h \
           codechunk.h \
           codemarker.h \
           codeparser.h \
           config.h \
           cppcodemarker.h \
           cppcodeparser.h \
           doc.h \
           editdistance.h \
           generator.h \
           helpprojectwriter.h \
           htmlgenerator.h \
           location.h \
           node.h \
           openedlist.h \
           plaincodemarker.h \
           puredocparser.h \
           qdocdatabase.h \
           qdoctagfiles.h \
           qdocindexfiles.h \
           quoter.h \
           separator.h \
           text.h \
           tokenizer.h \
           tree.h \
           webxmlgenerator.h

SOURCES += atom.cpp \
           clangcodeparser.cpp \
           codechunk.cpp \
           codemarker.cpp \
           codeparser.cpp \
           config.cpp \
           cppcodemarker.cpp \
           cppcodeparser.cpp \
           doc.cpp \
           editdistance.cpp \
           generator.cpp \
           helpprojectwriter.cpp \
           htmlgenerator.cpp \
           location.cpp \
           main.cpp \
           node.cpp \
           openedlist.cpp \
           plaincodemarker.cpp \
           puredocparser.cpp \
           qdocdatabase.cpp \
           qdoctagfiles.cpp \
           qdocindexfiles.cpp \
           quoter.cpp \
           separator.cpp \
           text.cpp \
           tokenizer.cpp \
           tree.cpp \
           yyindent.cpp \
           webxmlgenerator.cpp

### QML/JS Parser ###

HEADERS += jscodemarker.h \
            qmlcodemarker.h \
            qmlcodeparser.h \
            qmlmarkupvisitor.h \
            qmlvisitor.h

SOURCES += jscodemarker.cpp \
            qmlcodemarker.cpp \
            qmlcodeparser.cpp \
            qmlmarkupvisitor.cpp \
            qmlvisitor.cpp

### Documentation for qdoc ###

qtPrepareTool(QDOC, qdoc)
qtPrepareTool(QHELPGENERATOR, qhelpgenerator)

QMAKE_DOCS = $$PWD/doc/config/qdoc.qdocconf

QMAKE_TARGET_DESCRIPTION = "Qt Documentation Compiler"
load(qt_tool)

TR_EXCLUDE += $$PWD/*
