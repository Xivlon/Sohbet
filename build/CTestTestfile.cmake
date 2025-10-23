# CMake generated Testfile for 
# Source directory: /home/runner/work/Sohbet/Sohbet
# Build directory: /home/runner/work/Sohbet/Sohbet/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UserTest "/home/runner/work/Sohbet/Sohbet/build/test_user")
set_tests_properties(UserTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Sohbet/Sohbet/CMakeLists.txt;53;add_test;/home/runner/work/Sohbet/Sohbet/CMakeLists.txt;0;")
add_test(UserRepositoryTest "/home/runner/work/Sohbet/Sohbet/build/test_user_repository")
set_tests_properties(UserRepositoryTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Sohbet/Sohbet/CMakeLists.txt;57;add_test;/home/runner/work/Sohbet/Sohbet/CMakeLists.txt;0;")
add_test(BcryptTest "/home/runner/work/Sohbet/Sohbet/build/test_bcrypt")
set_tests_properties(BcryptTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Sohbet/Sohbet/CMakeLists.txt;61;add_test;/home/runner/work/Sohbet/Sohbet/CMakeLists.txt;0;")
add_test(AuthenticationTest "/home/runner/work/Sohbet/Sohbet/build/test_authentication")
set_tests_properties(AuthenticationTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Sohbet/Sohbet/CMakeLists.txt;65;add_test;/home/runner/work/Sohbet/Sohbet/CMakeLists.txt;0;")
subdirs("_deps/bcrypt-build")
