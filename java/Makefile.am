SUBDIRS = com
scrollview_path = @datadir@/tessdata

JAVAC = javac
JAR = jar

if !GRAPHICS_DISABLED
SCROLLVIEW_FILES = \
	$(srcdir)/com/google/scrollview/ui/SVAbstractMenuItem.java \
	$(srcdir)/com/google/scrollview/ui/SVCheckboxMenuItem.java \
	$(srcdir)/com/google/scrollview/ui/SVEmptyMenuItem.java \
	$(srcdir)/com/google/scrollview/events/SVEvent.java \
	$(srcdir)/com/google/scrollview/events/SVEventHandler.java \
	$(srcdir)/com/google/scrollview/events/SVEventType.java \
	$(srcdir)/com/google/scrollview/ui/SVImageHandler.java \
	$(srcdir)/com/google/scrollview/ui/SVMenuBar.java \
	$(srcdir)/com/google/scrollview/ui/SVMenuItem.java \
	$(srcdir)/com/google/scrollview/ui/SVPopupMenu.java \
	$(srcdir)/com/google/scrollview/ui/SVSubMenuItem.java \
	$(srcdir)/com/google/scrollview/ui/SVWindow.java \
	$(srcdir)/com/google/scrollview/ScrollView.java

SCROLLVIEW_CLASSES = \
	$(srcdir)/com/google/scrollview/ui/SVAbstractMenuItem.class \
	$(srcdir)/com/google/scrollview/ui/SVCheckboxMenuItem.class \
	$(srcdir)/com/google/scrollview/ui/SVEmptyMenuItem.class \
	$(srcdir)/com/google/scrollview/events/SVEvent.class \
	$(srcdir)/com/google/scrollview/events/SVEventHandler.class \
	$(srcdir)/com/google/scrollview/events/SVEventType.class \
	$(srcdir)/com/google/scrollview/ui/SVImageHandler.class \
	$(srcdir)/com/google/scrollview/ui/SVMenuBar.class \
	$(srcdir)/com/google/scrollview/ui/SVMenuItem.class \
	$(srcdir)/com/google/scrollview/ui/SVPopupMenu.class \
	$(srcdir)/com/google/scrollview/ui/SVSubMenuItem.class \
	$(srcdir)/com/google/scrollview/ui/SVWindow.class \
	$(srcdir)/com/google/scrollview/ScrollView.class

SCROLLVIEW_LIBS = \
	$(srcdir)/piccolo-1.2.jar \
	$(srcdir)/piccolox-1.2.jar

CLASSPATH = $(srcdir)/piccolo-1.2.jar:$(srcdir)/piccolox-1.2.jar

ScrollView.jar : $(SCROLLVIEW_CLASSES)
	$(JAR) cf $@ $(srcdir)/com/google/scrollview/*.class \
           $(srcdir)/com/google/scrollview/events/*.class $(srcdir)/com/google/scrollview/ui/*.class

$(SCROLLVIEW_CLASSES) : $(SCROLLVIEW_FILES)
	$(JAVAC) -encoding UTF8 -classpath $(CLASSPATH) $(SCROLLVIEW_FILES) 

.PHONY: install-jars
install-jars : ScrollView.jar
	@if [ ! -d  $(scrollview_path) ]; then mkdir -p $(scrollview_path); fi;
	$(INSTALL) -m 644 $(srcdir)/*.jar $(scrollview_path);
	$(INSTALL) -m 644 ScrollView.jar $(scrollview_path);
	@echo "Don't forget to set eviroment variable SCROLLVIEW_PATH to $(scrollview_path)";

uninstall:
	rm -f $(scrollview_path)/*.jar
endif

clean :
	rm -f ScrollView.jar *.class

# all-am does nothing, to make the java part optional.
all all-am install :
