import copy
import plearn.utilities.cvs                as     cvs
import plearn.utilities.plpath             as     plpath

from   programs                            import *
from   test_and_routines                   import *
from   ModeAndOptionParser                 import *
from   plearn.tasks.dispatch               import *
from   plearn.utilities.cvs                import *
from   plearn.utilities.toolkit            import *
from   plearn.utilities.verbosity          import *
from   plearn.utilities.global_variables   import *

current_mode    = None

dispatch        = None
targets         = None
options         = None

__all__ = [
    ## Functions
    'add_supported_modes',

    ## Variables
    'current_mode', 'dispatch', 'targets', 'options'        
    ]

## All modes that are completed must, in the class declaration,
## 
__supported_modes = [ 'add',  'commit', 'compile', 'disable', 'enable',
                      'list', 'ignore', 'results', 'run'                ]

def add_supported_modes( parser ):
    for mode in __supported_modes:
        mode_instance = eval( "%s()" % mode )
        parser.add_mode( mode_instance )
    
def print_stats():
    vprint("\n%s" % str(Test.statistics), 0)    

class PyTestMode(Mode):

    ## Static method
    def build_tests(none, directory, dirlist):
        if ignore.is_ignored(directory, dirlist):
            toolkit.exempt_list_of( dirlist, copy.copy(dirlist) )
            vprint( "Directory %s and subdirectories ignored.\n" % directory ) 
            return

        toolkit.exempt_list_of( dirlist, plpath.special_directories )    
        os.chdir(directory)

        config = config_file_path()
        try:
            if config in dirlist:
                execfile(config)
        except Exception, e:
            vprint( "+++ In %s" % os.path.join(directory, config) )
            if options.traceback:
                raise
            else:
                vprint( "%s: %s." % (e.__class__.__name__,e) )
    build_tests = staticmethod(build_tests)

    ## Class methods
    def parse_config_files(cls):
        for target in targets:
            if hasattr(options, 'recursive') and options.recursive:
                os.path.walk(target, cls.build_tests, None)
            else:
                cls.build_tests(None, target, os.listdir(target))
    parse_config_files = classmethod(parse_config_files)

           
    ## Instance methods
    def __init__(self):
        Mode.__init__(self)

    def option_groups(self, parser):
        return []
    
    def procedure(self):
        raise NotImplementedError

    def run(self):
        try:
            self.procedure()
        except PyTestUsageError, e: 
            if options.traceback:
                raise
            else:
                vprint( "%s: %s." % (e.__class__.__name__,e) )

    def testing_options(self, parser):
        testing_options = OptionGroup( parser, "Testing Options", "" )

        testing_options.add_option( "--all",
                                    action="store_true", default=False,
                                    help= "Run all tests found in subdirectories of directories in "
                                    "globalvars.all_roots test_suite branches. If some targets "
                                    "are provided, these will be ignored. " 
                                    )

        testing_options.add_option( "-R", "--recursive",
                                    action="store_true", default=False,
                                    help = 'Process all targets recursively. If some target is '
                                    'the subdirectory to another target, it will be ignored, i.e. '
                                    'the whole hierarchy will be tested only once.'
                                    ) 

        testing_options.add_option( '-l', '--localhost',
                                    action='store_true',
                                    help='This flag triggers a dispatch using only the local host. '
                                    'CURRENTLY ALWAYS TRUE!!!',
                                    default=True )

        testing_options.add_option( '--hosts', 
                                    help='The maximum nuber of hosts to use simultaneously.',
                                    default=10 )

        testing_options.add_option( '-n', '--test-name',
                                    help='Restricts the current mode to the named test.',
                                    default='' )

        testing_options.add_option( '--traceback',
                                    action="store_true",
                                    help="This flag triggers routines to report the traceback of "
                                    "PyTestUsageError. By default, only the class's name and meesage "
                                    "are reported.",
                                    default = False )
        
        return testing_options

class FamilyConfigMode( PyTestMode ):
    def procedure(self):
        initialize()
        self.parse_config_files()

        for (family, tests) in Test.families_map.iteritems():
            config_path  = config_file_path( family )
            if os.path.exists( config_path ):
                os.system( "cp %s %s.%s"
                           % ( config_path, config_path,
                               toolkit.date_time_string())
                           )

            config_file  = open(config_path, 'w')

            config_text  = ( '"""Pytest config file.\n\n%s\n"""'% toolkit.doc(Test) )

            for test in tests:
                self.test_hook( test ) 
                config_text += str( test ) + '\n'

            config_file.write(config_text)
            config_file.close()    

    def test_hook(self, test):
        raise NotImplementedError
    
    def option_groups(self, parser):
        return [ self.testing_options(parser) ]

class add( FamilyConfigMode ):
    """Adds a test in a given directory.
    
    A directory is considered to be a test directory as soon as it
    contains a I{pytest.config} file. Therefore, this mode simply
    drops a config file with some explanation how to instanciate the
    L{Test} objects within the file. If a I{pytest.config} file
    already exists, it is appended a new L{Test} template.
    
    The config file is executed within the pytest script. So, it B{is}
    in python, which means that one can add any comments he wishes and
    may also define own functions if complicated processing is
    requested before defining the test.
    """
    
    ## Static method
    def build_tests(none, directory, dirlist):
        PyTestMode.build_tests(none, directory, dirlist)

        test_name = options.test_name
        if test_name == '':
            test_name = '<MANDATORY_TEST_NAME_%s>' % toolkit.date_time_string()

        Test( name    = test_name,
              program = GlobalCompilableProgram( name = "plearn")  )        
    build_tests = staticmethod(build_tests)

    def test_hook(self, test):
        pass
    
class commit( PyTestMode ):
    """Commits PyTest's config file and results within the target directory."""
    def procedure( self ):
        global targets
        if len(targets) == 0:
            targets = [ os.getcwd() ]
        else:
            targets = plpath.exempt_of_subdirectories( targets )            
        self.parse_config_files()

        for (family, tests) in Test.families_map.iteritems():
            os.chdir( family )

            if not os.path.exists( plpath.cvs_directory ):
                raise PyTestUsageError(
                    "The directory in which lies a config file must be added to cvs by user.\n"
                    "%s was not." % family
                    )

            config_path = config_file_path()

            cvs.add( config_path )
            cvs.add( plpath.pytest_dir )
            cvs.add( Test.expected_results )
            for test in tests:
                cvs.recursive_add( test.test_results( Test.expected_results ) )

            cvs.commit( [config_path, plpath.pytest_dir],
                        'PyTest internal commit'
                        )            

class disable(FamilyConfigMode):
    """Disables targeted tests.

    The disabled tests can be restored (L{enable mode<enable>}) afterwards.
    """
    def test_hook( self, test ):
        test.disabled = True

class enable( FamilyConfigMode ):
    """Enables disabled (L{disable mode<mode_disable>}) tests."""
    def test_hook( self, test ):
        test.disabled = False

class RoutineBasedMode( PyTestMode ):

    def description(self):
        return toolkit.doc( self.routine_type() )

    def dispatch_tests(self, test_instances):
        for (test_name, test) in test_instances:
            RoutineType = self.routine_type()
            dispatch.add_task( RoutineType(test) )
        dispatch.run()

    def help(self):
        return toolkit.short_doc( self.routine_type() )

    def procedure(self):
        initialize()
        
        ## --traceback: This flag triggers routines to report the traceback of
        ## PyTestUsageError. By default, only the class's name and meesage
        ## are reported.
        Routine.report_traceback = options.traceback

        self.parse_config_files()

        test_instances = None
        if options.test_name:
            test_instances = [( options.test_name,
                                Test.instances_map[ options.test_name ]
                                )]
        else:
            test_instances = Test.instances_map.items()

        self.dispatch_tests( test_instances )
        print_stats()

    def routine_type(self):
        raise NotImplementedError
    
    def option_groups(self, parser):
        return [ self.testing_options(parser) ]

class compile( RoutineBasedMode ):
    def routine_type(self): return CompilationRoutine

class results( RoutineBasedMode ):
    def routine_type(self): return ResultsCreationRoutine

class run( RoutineBasedMode ):
    def routine_type(self): return RunTestRoutine

def initialize( ):
    global dispatch, targets

    dispatch = Dispatch(
        localhost = options.localhost,
        nb_hosts  = options.hosts
        )
    
    ## --all: Run all tests found in subdirectories of directories in
    ## globalvars.all_roots test_suite branches. If some targets are
    ## provided, these will be ignored.
    if options.all:
        targets           = globalvars.all_roots
        options.recursive = True

    ## If no targets are provided, the cwd is the default target.
    if len(targets) == 0:
        targets.append( os.getcwd() )

    if options.recursive:
        targets = plpath.exempt_of_subdirectories( targets )


class list(PyTestMode):
    """Lists all tests within target directories."""
    def procedure(self):
        initialize()
        self.parse_config_files()

        formatted_string = lambda n,d: ( "%s Disabled: %s"
                                         % (string.ljust(n, 25), string.ljust(str(d), 15))
                                         )
        for (family, tests) in Test.families_map.iteritems():
            formatted_strings = []

            for test in tests:
                if options.disabled and not test.disabled:
                    continue
                if options.enabled  and test.disabled:
                    continue
                formatted_strings.append(
                    formatted_string(test.name, test.disabled)
                    )

            if formatted_strings:
                vprint( "In %s:\n    %s\n"
                        % ( family, string.join(formatted_strings, '\n    ') )
                        )    
        
    def option_groups( self, parser ):
        list_options = OptionGroup( parser, "Mode Specific Options --- %s" % self.classname(),
                                    "Available under list mode only." )

        list_options.add_option( "-d", "--disabled",
                                 action="store_true",
                                 default=False,
                                 help= "The list provided will contain only disabled tests." 
                                 )

        list_options.add_option( "-e", "--enabled",
                                 action="store_true",
                                 default=False,
                                 help= "The list provided will contain only enabled tests." 
                                 )

        return [ self.testing_options(parser), list_options ]

class ignore(PyTestMode):
    """Causes the target hierarchy to be ignored by PyTest.

    Simply drops a I{pytest.ignore} file that is recognized by PyTest
    afterwards.
    """
    ignore_file               = 'pytest.ignore'

    def is_ignored(directory, dirlist=None):
        if dirlist == None:
            dirlist = os.listdir(directory)
            
        if ignore.ignore_file in dirlist:
            return True
        return False
    is_ignored = staticmethod(is_ignored)

    def procedure(self):
        global targets
        if len(targets) == 0:
            targets = [ os.getcwd() ]
        else:
            targets = plpath.exempt_of_subdirectories( targets )

        for target in targets:
            os.system("touch %s" % os.path.join( target, ignore.ignore_file ) )
        
