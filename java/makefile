JAVAC = javac
JAR = jar

SCROLLVIEW_FILES = \
SVAbstractMenuItem.java \
SVCheckboxMenuItem.java \
SVEmptyMenuItem.java \
SVEvent.java \
SVEventHandler.java \
SVEventType.java \
SVImageHandler.java \
SVMenuBar.java \
SVMenuItem.java \
SVPopupMenu.java \
SVSubMenuItem.java \
SVWindow.java \
ScrollView.java

SCROLLVIEW_CLASSES = \
SVAbstractMenuItem.class \
SVCheckboxMenuItem.class \
SVEmptyMenuItem.class \
SVEvent.class \
SVEventHandler.class \
SVEventType.class \
SVImageHandler.class \
SVMenuBar.class \
SVMenuItem.class \
SVPopupMenu.class \
SVSubMenuItem.class \
SVWindow.class \
ScrollView.class

SCROLLVIEW_LIBS = \
piccolo-1.2.jar \
piccolox-1.2.jar

CLASSPATH = ./piccolo-1.2.jar:./piccolox-1.2.jar

ScrollView.jar : $(SCROLLVIEW_CLASSES)
	$(JAR) -c -f $@ $(SCROLLVIEW_CLASSES)

$(SCROLLVIEW_CLASSES) : $(SCROLLVIEW_FILES)
	$(JAVAC) -encoding UTF8 -classpath $(CLASSPATH) $(SCROLLVIEW_FILES) 

clean :
	rm -f ScrollView.jar *.class

# all-am does nothing, to make the java part optional.
all all-am install :

# dist runs the autoconf makefile to archive the files correctly.
dist distdir :
	make -f Makefile $@
