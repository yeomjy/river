import glob
import os
import shutil
import subprocess

from fuzzers import utils

def build():
    # Environment variables
    os.environ['CC'] = 'clang'    # C compiler.
    os.environ['CXX'] = 'clang++' # C++ compiler.
    os.environ['FUZZER_LIB'] = '/workspace/river_shim.o' # River shim library

    utils.build_benchmark()

    shutil.copytree('/workspace/river', os.environ['OUT'] + "/river", dirs_exist_ok=True)
    shutil.copytree('/usr/local/lib/python3.8', os.environ['OUT'] + "/lib/python3.8/", dirs_exist_ok=True)
    shutil.copytree('/usr/local/include/python3.8', os.environ['OUT'] + "/include/python3.8/", dirs_exist_ok=True)

    shutil.copy('/usr/local/lib/libtriton.so', os.environ['OUT'] + "/lib/")
    for f in glob.glob('/usr/local/lib/libpython3*'):
        shutil.copy(f, os.environ['OUT'] + "/lib/")

    print('[post_build] Finished river building process')

def prepare_fuzz_environment():
    """Add path to pip installed libraries."""
    os.environ['LD_LIBRARY_PATH'] = '{0}/lib/:{0}/lib/python3.8/site-packages/z3/lib/:{0}/lib/python3.8/site-packages/capstone/lib/'.format(os.environ['OUT'])
    os.environ['PYTHONPATH'] = '{0}/lib/python3.8/site-packages/'.format(os.environ['OUT'])

def fuzz(input_corpus, output_corpus, target_binary):
    """Run fuzzer."""
    # Seperate out corpus and crash directories as sub-directories of
    # |output_corpus| to avoid conflicts when corpus directory is reloaded.
    crashes_dir = os.path.join(output_corpus, 'crashes')
    output_corpus = os.path.join(output_corpus, 'corpus')
    os.makedirs(crashes_dir)
    os.makedirs(output_corpus)

    prepare_fuzz_environment()

    print('[fuzz] Running target with river')
    command = [
        'python3', '/out/river/River3/python/concolic_GenerationalSearch2.py',
        '--binaryPath', target_binary,
        '--architecture', 'x64',
        '--maxLen', '10',
        '--logLevel', 'CRITICAL',
        '--secondsBetweenStats', '10',
        '--outputType', 'textual',
        '--entryfuncName', 'main',
        #'--input', input_corpus,
        #'--output', output_corpus,
        #'--crashdir', crashes_dir,
    ]
    dictionary_path = utils.get_dictionary_path(target_binary)
    if dictionary_path:
        command.extend(['--dict', dictionary_path])

    print('[fuzz] Running command: ' + ' '.join(command))
    subprocess.check_call(command)
