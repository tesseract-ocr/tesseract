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
	piccolo2d-core-3.0.jar \
	piccolo2d-extras-3.0.jar

CLASSPATH = piccolo2d-core-3.0.jar:piccolo2d-extras-3.0.jar

ScrollView.jar : $(SCROLLVIEW_CLASSES)
	$(JAR) cfm $@ $(srcdir)/Manifest.txt com/google/scrollview/*.class \
           com/google/scrollview/events/*.class com/google/scrollview/ui/*.class

$(SCROLLVIEW_CLASSES) : $(SCROLLVIEW_FILES) $(SCROLLVIEW_LIBS)
	$(JAVAC) -encoding UTF8 -sourcepath $(srcdir) -classpath $(CLASSPATH) $(SCROLLVIEW_FILES) -d $(builddir)

.PHONY: fetch-jars
fetch-jars $(SCROLLVIEW_LIBS):
	curl -L http://search.maven.org/remotecontent?filepath=org/piccolo2d/piccolo2d-core/3.0/piccolo2d-core-3.0.jar > piccolo2d-core-3.0.jar
	curl -L http://search.maven.org/remotecontent?filepath=org/piccolo2d/piccolo2d-extras/3.0/piccolo2d-extras-3.0.jar > piccolo2d-extras-3.0.jar

.PHONY: install-jars
install-jars : ScrollView.jar
	@if [ ! -d  $(scrollview_path) ]; then mkdir -p $(scrollview_path); fi;
	$(INSTALL) -m 644 $(SCROLLVIEW_LIBS) $(scrollview_path);
	$(INSTALL) -m 644 ScrollView.jar $(scrollview_path);
	@echo "Don't forget to set environment variable SCROLLVIEW_PATH to $(scrollview_path)";

uninstall:
	rm -f $(scrollview_path)/*.jar
endif

clean-local:
	rm -f ScrollView.jar $(SCROLLVIEW_CLASSES)

# all-am does nothing, to make the java part optional.
all all-am install :
