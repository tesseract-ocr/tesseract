:version: $RCSfile: index.rst,v $ $Revision: 76e0bf38aaba $ $Date: 2011/03/22 00:48:41 $

.. default-role:: fs

==================================
 Maintaining the VS2008 directory
==================================

This section is geared towards project maintainers of the
`tesseract-3.0x\\vs2008` directory, rather than users of it.

Python 2.7.x (*not* 3.x) is required for this section. The recommended
version is the `latest from ActiveState
<http://www.activestate.com/activepython/downloads>`_.
 
.. _tesshelper:

The `tesshelper.py` Python script
=================================

`tesshelper.py` performs a number of useful maintenance related
operations on the `tesseract-3.0x\\vs2008` directory. To run it, first
open a Command Prompt window and navigate to the `<tesseract install
dir>\\vs2008` directory.

Then entering the following command::

   python tesshelper.py --help

displays the following help message::

   usage: tesshelper.py [-h] [--version] tessDir {compare,report,copy,clean} ...

   positional arguments:
     tessDir               tesseract installation directory

   optional arguments:
     -h, --help            show this help message and exit
     --version             show program's version number and exit

   Commands:
     {compare,report,copy,clean}
       compare             compare libtesseract Project with tessDir
       report              report libtesseract summary stats
       copy                copy public libtesseract header files to includeDir
       clean               clean vs2008 folder of build folders and .user files

   Examples:

   Assume that tesshelper.py is in c:\buildfolder\tesseract-3.01\vs2008,
   which is also the current directory. Then,

       python tesshelper .. compare

   will compare c:\buildfolder\tesseract-3.01 "library" directories to the
   libtesseract Project
   (c:\buildfolder\tesseract-3.01\vs2008\libtesseract\libtesseract.vcproj).

       python tesshelper ..  report

   will display summary stats for c:\buildfolder\tesseract-3.01 "library"
   directories and the libtesseract Project.

       python tesshelper .. copy ..\..\include

   will copy all "public" libtesseract header files to
   c:\buildfolder\include.

       python tesshelper .. clean

   will clean the vs2008 folder of all build directories, and .user, .suo,
   .ncb, and other temp files.

Generating the documentation
============================

The source files for the documentation you are currently reading are
written in `reStructuredText
<http://docutils.sourceforge.net/rst.html>`_ and processed with the
`Sphinx Python Documentation Generator
<http://sphinx.pocoo.org/index.html>`_.

To install Sphinx, go to your `<python.2.7.x install dir>\\scripts`
directory and just do::

   easy_install -U Sphinx

which will download Sphinx and all its dependencies. [Note: This might
*not* install the Python Imaging Library. If not, then also do
``easy_install -U PIL`` or download it from `here
<http://www.pythonware.com/products/pil/>`__.]

To generate this |Tesseractocr| VS2008 documentation go to
`tesseract-3.0x\\vs2008\\Sphinx` and do::

   make clean
   make html

Which will create a number of items in
`tesseract-3.0x\\vs2008\\Sphinx\\_build\\html`.

Copy everything there to the distribution's `tesseract-3.0x\\vs2008\\doc`
folder, :bi:`except` for::

   .buildinfo
   objects.inv

.. _updating-vs2008-directory:

Updating the VS2008 directory for new releases of |Tesseractocr|
================================================================

1. Change the version number strings in
   `tesseract-3.0x\\vs2008\\include\\tesseract_versionnumbers.vsprops`.

#. Change the version number in
   `tesseract-3.0x\\vs2008\\port\\version.h`.

#. Open up a Command Prompt window, and do the following::

      cd <tesseract-3.0x install dir>\vs2008
      python tesshelper .. compare

   This will list all added and missing items in the `<tesseract-3.0x install
   dir>` directories that are used to build `libtesseract`. For the
   newly added items ignore::

      api\tesseractmain.cpp 
      api\tesseractmain.h 
      ccutil\scanutils.cpp 
      ccutil\scanutils.h 

   and for the newly missing items ignore::

      training\commontraining.cpp 
      training\commontraining.h 
      training\tessopt.cpp 
      training\tessopt.h 

#. Open up the `tesseract.sln` in Visual Studio 2008 (or Visual C++ 2008
   Express Edition but see :ref:`this 
   <building-with-vc2008-express>` first).

   a. In the Solution Explorer, rename the :guilabel:`libtesseract-3.0x`
      Project to the correct version number to make it obvious which
      version of |Tesseractocr| this Solution is for.

   #. Remove the missing items from the :guilabel:`libtesseract-3.0x` Project.

   #. Add the new items to the :guilabel:`libtesseract-3.0x` Project.

      If there were a lot of new items, you can use the `newheaders.txt`
      and `newsources.txt` files generated by running the
      `tesshelper.py` script with the ``compare`` command. Close the
      Solution, and then you can directly edit
      `libtesseract\\libtesseract.vcproj` to add them to the appropriate
      ``<Filter> ... </Filter>`` section (either ``Header Files`` or
      ``Source Files``).

#. With the Solution closed, use a text editor to change all the
   Project's `.rc` files to reflect the new version.

   If you have a program like the *non-free* `PowerGrep
   <http://www.powergrep.com/>`_, you can use it to change all the
   `.rc` files in one fell swoop.

   Alternatively, you can edit the Version resources within Visual
   Studio 2008 (but *not* Visual C++ 2008 Express Edition) and then
   manually make the changes mentioned :ref:`here
   <building-with-vc2008-express>` afterwards.

   .. _copying_a_project:

#. If a new training application was added (edit
   `tesseract-3.0x\\training\\Makefile.am` and look at the
   ``bin_PROGRAMS`` variable to see the list), the easiest thing to do
   is copy another existing training application Project and manually
   change it.

   For example, assuming the new training application is
   called `new_trainer.exe`, with the Solution closed:

   a. Copy the `ambiguous_words` directory to a new directory called
      `new_trainer`.

   #. Change the `new_trainer\\ambiguous_words.rc` filename to
      `new_trainer\\new_trainer.rc`.

   #. Change the `new_trainer\\ambiguous_words.vcproj` filename to
      `new_trainer\\new_trainer.vcproj`.

   #. Edit `new_trainer\\new_trainer.rc` and change all occurrences of
      ``ambiguous_words`` to ``new_trainer``.

      Also change ``FileDescription`` to describe the new application.

   #. Open up the |Tesseractocr| Solution file and right-click the
      :guilabel:`Solution:'tesseract'` in the Solution Explorer. Choose
      :menuselection:`A&dd --> &Existing Project...` from the context
      menu and add the `new_trainer\\new_trainer.vcproj` you just
      created.

   #. Right-click the newly added Project, and choose
      :menuselection:`Project Dependencie&s...`.

      The :guilabel:`Project Dependencies` Dialog will open. Make sure
      that `libtesseract30x` is checked. If you forget this step, Visual
      Studio will not automatically link with `libtesseract` and
      you'll get lots of "unresolved external symbol" errors.

   This actually goes pretty fast. It should only take you a minute or
   so to add a new application to the |Tesseractocr| Solution.

#. (Optional?) Edit `vs2008\\Sphinx\\versions.rst` and add a new entry
   describing the changes made for this new version.
   
#. To make your working directory suitable for reposting back to the
   |Tesseractocr| SVN repository, you need to ignore all the following:

   + All `LIB_Release`, `LIB_Debug`, `DLL_Release`, `DLL_Debug`
     directories
   + All `.suo` files
   + All `.user` files
   + All `.ncb` files
   + `vs2008\\newheaders.txt`
   + `vs2008\\newsources.txt`

   Optionally, the `tesshelper.py` script has the ``clean`` command
   which will remove the above items. To run it, open a Command Prompt
   window and then do::

      cd <tesseract-3.0x install dir>\vs2008
      python tesshelper .. clean

   The script will respond with the following::

      Are you sure you want to clean the
       "C:\BuildFolder\tesseract-3.0x\vs2008" folder (Yes/No) [No]? yes
      Only list the items to be deleted (Yes/No) [Yes]? no

   You have to answer ``yes`` and then ``no`` to the prompts. Otherwise
   either the script will just exit, or only list the items that will be
   removed instead of actually removing them (which is a good thing to
   try first just in case).

..         
   Local Variables:
   coding: utf-8
   mode: rst
   indent-tabs-mode: nil
   sentence-end-double-space: t
   fill-column: 72
   mode: auto-fill
   standard-indent: 3
   tab-stop-list: (3 6 9 12 15 18 21 24 27 30 33 36 39 42 45 48 51 54 57 60)
   End:
