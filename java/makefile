JAVAC = javac
JAR = jar

SCROLLVIEW_FILES = \
com/google/scrollview/ui/SVAbstractMenuItem.java \
com/google/scrollview/ui/SVCheckboxMenuItem.java \
com/google/scrollview/ui/SVEmptyMenuItem.java \
com/google/scrollview/events/SVEvent.java \
com/google/scrollview/events/SVEventHandler.java \
com/google/scrollview/events/SVEventType.java \
com/google/scrollview/ui/SVImageHandler.java \
com/google/scrollview/ui/SVMenuBar.java \
com/google/scrollview/ui/SVMenuItem.java \
com/google/scrollview/ui/SVPopupMenu.java \
com/google/scrollview/ui/SVSubMenuItem.java \
com/google/scrollview/ui/SVWindow.java \
com/google/scrollview/ScrollView.java

SCROLLVIEW_CLASSES = \
com/google/scrollview/ui/SVAbstractMenuItem.class \
com/google/scrollview/ui/SVCheckboxMenuItem.class \
com/google/scrollview/ui/SVEmptyMenuItem.class \
com/google/scrollview/events/SVEvent.class \
com/google/scrollview/events/SVEventHandler.class \
com/google/scrollview/events/SVEventType.class \
com/google/scrollview/ui/SVImageHandler.class \
com/google/scrollview/ui/SVMenuBar.class \
com/google/scrollview/ui/SVMenuItem.class \
com/google/scrollview/ui/SVPopupMenu.class \
com/google/scrollview/ui/SVSubMenuItem.class \
com/google/scrollview/ui/SVWindow.class \
com/google/scrollview/ScrollView.class

SCROLLVIEW_LIBS = \
piccolo-1.2.jar \
piccolox-1.2.jar

CLASSPATH = ./piccolo-1.2.jar:./piccolox-1.2.jar

ScrollView.jar : $(SCROLLVIEW_CLASSES)
	$(JAR) cf $@ com/google/scrollview/*.class \
           com/google/scrollview/events/*.class com/google/scrollview/ui/*.class

$(SCROLLVIEW_CLASSES) : $(SCROLLVIEW_FILES)
	$(JAVAC) -encoding UTF8 -classpath $(CLASSPATH) $(SCROLLVIEW_FILES) 

clean :
	rm -f ScrollView.jar *.class

# all-am does nothing, to make the java part optional.
all all-am install :

# dist runs the autoconf makefile to archive the files correctly.
dist distdir :
	make -f Makefile $@
uninstall uninstall-am:
	make -f Makefile $@

