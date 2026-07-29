//
// Created by chris on 10/15/2025.
//

#ifndef MLA_FILE_SYSTEM_TEST_H
#define MLA_FILE_SYSTEM_TEST_H

#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/core/filesystem/mla_file_system.h"

mla_bool_t mla_file_system_test_write_string(const mla_string_t& path, const mla_string_t& content) {
    mla_file_system_stream_t stream = mla_file_system_stream_empty();

    if (!mla_fs_open_file(path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream) || stream.write == nullptr) {
        return false;
    }

    mla_size_t length = mla_string_length(content);
    return stream.write(stream, 0, length, mla_r_cast<const mla_byte_t*>(mla_string_data(content))) == length;
}

mla_string_t mla_file_system_test_read_string(const mla_string_t& path) {
    mla_file_system_stream_t stream = mla_file_system_stream_empty();

    if (!mla_fs_open_file(path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, stream) || stream.length == nullptr) {
        return mla_string_empty();
    }

    mla_stream_input_t input = mla_file_system_stream_as_input(stream);
    return mla_string_from_stream(input, stream.length(stream) + 1);
}

void FileSystemIsDirectoryPathTest() {
    // Valid directory paths (must start and end with slash)
    assert_true(mla_fs_is_directory_path(mla_string("/")),
                "Root directory should be valid");
    assert_true(mla_fs_is_directory_path(mla_string("/test/")),
                "Simple directory should be valid");
    assert_true(mla_fs_is_directory_path(mla_string("/test/nested/")),
                "Nested directory should be valid");

    // Invalid directory paths
    assert_false(mla_fs_is_directory_path(mla_string("test/")),
                 "Path without leading slash should be invalid");
    assert_false(mla_fs_is_directory_path(mla_string("/test")),
                 "Path without trailing slash should be invalid");
    assert_false(mla_fs_is_directory_path(mla_string("test")),
                 "Path without slashes should be invalid");
    assert_false(mla_fs_is_directory_path(mla_string("")),
                 "Empty path should be invalid");
    assert_false(mla_fs_is_directory_path(mla_string("test/dummy.txt")),
                 "File path should be invalid");
}

void FileSystemGetParentDirectoryTest() {
    // Standard cases
    mla_string_t parent = mla_fs_get_parent_directory(mla_string("/test/dummy.txt"));
    assert_true(mla_string_equals(parent, mla_string("/test/")),
                "Parent of 'test/dummy.txt' should be 'test/'");

    parent = mla_fs_get_parent_directory(mla_string("/test/nested/file.txt"));
    assert_true(mla_string_equals(parent, mla_string("/test/nested/")),
                "Parent of 'test/nested/file.txt' should be '/test/nested/'");

    parent = mla_fs_get_parent_directory(mla_string("/root/file.txt"));
    assert_true(mla_string_equals(parent, mla_string("/root/")),
                "Parent of '/root/file.txt' should be '/root/'");

    // Edge cases
    parent = mla_fs_get_parent_directory(mla_string("file.txt"));
    assert_true(mla_string_equals(parent, mla_string_const("/")),
                "Parent of 'file.txt' should be '/'");

    parent = mla_fs_get_parent_directory(mla_string(""));
    assert_true(mla_string_equals(parent, mla_string_empty()),
                "Parent of empty string should be empty");

    parent = mla_fs_get_parent_directory(mla_string("/"));
    assert_true(mla_string_equals(parent, mla_string_const("/")),
                "Parent of root '/' should be '/'");
}

void FileSystemGetFileNameTest() {
    // Standard cases
    mla_string_t filename = mla_fs_get_file_name(mla_string("/test/dummy.txt"));
    assert_true(mla_string_equals(filename, mla_string("dummy.txt")),
                "Filename of 'test/dummy.txt' should be 'dummy.txt'");

    filename = mla_fs_get_file_name(mla_string("/test/nested/file.bin"));
    assert_true(mla_string_equals(filename, mla_string("file.bin")),
                "Filename of 'test/nested/file.bin' should be 'file.bin'");

    filename = mla_fs_get_file_name(mla_string("/root/document.pdf"));
    assert_true(mla_string_equals(filename, mla_string("document.pdf")),
                "Filename of '/root/document.pdf' should be 'document.pdf'");

    // Edge cases
    filename = mla_fs_get_file_name(mla_string("/standalone.txt"));
    assert_true(mla_string_equals(filename, mla_string("standalone.txt")),
                "Filename with root path should return whole string");

    filename = mla_fs_get_file_name(mla_string("standalone.txt"));
    assert_true(mla_string_equals(filename, mla_string("standalone.txt")),
                "Filename without path should return whole string");

    filename = mla_fs_get_file_name(mla_string(""));
    assert_true(mla_string_equals(filename, mla_string_empty()),
                "Filename of empty string should be empty");
}

void FileSystemGetFileExtensionTest() {
    // Standard cases
    mla_string_t ext = mla_fs_get_file_extension(mla_string("/test/dummy.txt"));
    assert_true(mla_string_equals(ext, mla_string("txt")),
                "Extension of 'test/dummy.txt' should be 'txt'");

    ext = mla_fs_get_file_extension(mla_string("/test/file.doc.bak"));
    assert_true(mla_string_equals(ext, mla_string("bak")),
                "Extension of 'test/file.doc.bak' should be 'bak'");

    ext = mla_fs_get_file_extension(mla_string("/root/image.PNG"));
    assert_true(mla_string_equals(ext, mla_string("PNG")),
                "Extension of '/root/image.PNG' should be 'PNG'");

    // Edge cases
    ext = mla_fs_get_file_extension(mla_string("/test/noextension"));
    assert_true(mla_string_equals(ext, mla_string_empty()),
                "File without extension should return empty");

    ext = mla_fs_get_file_extension(mla_string("/test/.hidden"));
    assert_true(mla_string_equals(ext, mla_string("hidden")),
                "Hidden file '.hidden' should return 'hidden' as extension");

    ext = mla_fs_get_file_extension(mla_string(""));
    assert_true(mla_string_equals(ext, mla_string_empty()),
                "Empty path should return empty extension");
}

void FileSystemChangeFileExtensionTest() {
    // Standard cases
    mla_string_t newPath = mla_fs_change_file_extension(mla_string("/test/dummy.txt"), mla_string("md"));
    assert_true(mla_string_equals(newPath, mla_string("/test/dummy.md")),
                "Changing 'test/dummy.txt' to 'md' should give '/test/dummy.md'");

    newPath = mla_fs_change_file_extension(mla_string("/test/file.doc.bak"), mla_string("tmp"));
    assert_true(mla_string_equals(newPath, mla_string("/test/file.doc.tmp")),
                "Changing 'test/file.doc.bak' to 'tmp' should give '/test/file.doc.tmp'");

    // Remove extension
    newPath = mla_fs_change_file_extension(mla_string("/test/dummy.txt"), mla_string_empty());
    assert_true(mla_string_equals(newPath, mla_string("/test/dummy")),
                "Removing extension from 'test/dummy.txt' should give 'test/dummy'");

    // File without extension
    newPath = mla_fs_change_file_extension(mla_string("/test/noext"), mla_string("bin"));
    assert_true(mla_string_equals(newPath, mla_string("/test/noext.bin")),
                "Adding extension to 'test/noext' should give 'test/noext.bin'");

    // No parent directory
    newPath = mla_fs_change_file_extension(mla_string("/file.txt"), mla_string("log"));
    assert_true(mla_string_equals(newPath, mla_string("/file.log")),
                "Changing 'file.txt' to 'log' should give 'file.log'");
}

void FileSystemCombinePathsTest() {
    // Standard cases
    mla_string_t combined = mla_fs_combine_paths(mla_string("test"), mla_string("dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/test/dummy.txt")),
                "Combining 'test' and 'dummy.txt' should give '/test/dummy.txt'");

    combined = mla_fs_combine_paths(mla_string("test/"), mla_string("dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/test/dummy.txt")),
                "Combining 'test/' and 'dummy.txt' should give '/test/dummy.txt'");

    combined = mla_fs_combine_paths(mla_string("test"), mla_string("/dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/test/dummy.txt")),
                "Combining 'test' and '/dummy.txt' should give '/test/dummy.txt'");

    combined = mla_fs_combine_paths(mla_string("test/"), mla_string("/dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/test/dummy.txt")),
                "Combining 'test/' and '/dummy.txt' should give '/test/dummy.txt'");

    // Edge cases
    combined = mla_fs_combine_paths(mla_string(""), mla_string("dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/dummy.txt")),
                "Combining empty and 'dummy.txt' should give '/dummy.txt'");

    combined = mla_fs_combine_paths(mla_string("test"), mla_string(""));
    assert_true(mla_string_equals(combined, mla_string("/test")),
                "Combining 'test' and empty should give 'test'");

    // Multiple levels
    combined = mla_fs_combine_paths(mla_string("root/nested"), mla_string("file.txt"));
    assert_true(mla_string_equals(combined, mla_string("/root/nested/file.txt")),
                "Combining 'root/nested' and 'file.txt' should give 'root/nested/file.txt'");

}

void FileSystemFileExistsTest() {
    // Test non-existent file
    assert_false(mla_fs_file_exists(mla_string("/test/nonexistent.txt")),
                 "Non-existent file should return false");

    // Create a directory and file for testing
    assert_true(mla_fs_create_directory(mla_string("/testdir/")),
                "Should create test directory");

    mla_file_system_stream_t stream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/testdir/test.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream),
                "Should create test file");
    // Close the stream
    stream = mla_file_system_stream_empty();
    (void)stream;

    // Test existing file
    assert_true(mla_fs_file_exists(mla_string("/testdir/test.txt")),
                "Created file should exist");

    // Cleanup
    mla_fs_delete_file(mla_string("/testdir/test.txt"));
    mla_fs_delete_directory(mla_string("/testdir/"));
}

void FileSystemDeleteFileTest() {
    // Create test file
    assert_true(mla_fs_create_directory(mla_string("/deltest/")),
                "Should create test directory");

    mla_file_system_stream_t stream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/deltest/todelete.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream),
                "Should create file to delete");
    // Close the stream
    stream = mla_file_system_stream_empty();
    (void)stream;

    // Verify file exists
    assert_true(mla_fs_file_exists(mla_string("/deltest/todelete.txt")),
                "File should exist before deletion");

    // Delete file
    assert_true(mla_fs_delete_file(mla_string("/deltest/todelete.txt")),
                "Should successfully delete file");

    // Verify file no longer exists
    assert_false(mla_fs_file_exists(mla_string("/deltest/todelete.txt")),
                 "File should not exist after deletion");

    // Try deleting non-existent file
    assert_false(mla_fs_delete_file(mla_string("/deltest/nonexistent.txt")),
                 "Deleting non-existent file should fail");

    // Cleanup
    mla_fs_delete_directory(mla_string("/deltest/"));
}

void FileSystemDirectoryExistsTest() {
    // Test non-existent directory
    assert_false(mla_fs_directory_exists(mla_string("/nonexistentdir/")),
                 "Non-existent directory should return false");

    // Create directory
    assert_true(mla_fs_create_directory(mla_string("/existingdir/")),
                "Should create test directory");

    // Test existing directory
    assert_true(mla_fs_directory_exists(mla_string("/existingdir/")),
                "Created directory should exist");

    // Cleanup
    mla_fs_delete_directory(mla_string("/existingdir/"));
}

void FileSystemCreateDirectoryTest() {
    // Create simple directory
    assert_true(mla_fs_create_directory(mla_string("/newdir/")),
                "Should create new directory");
    assert_true(mla_fs_directory_exists(mla_string("/newdir/")),
                "Created directory should exist");

    // Create nested directory
    assert_true(mla_fs_create_directory(mla_string("/parent/")),
                "Should create parent directory");
    assert_true(mla_fs_create_directory(mla_string("/parent/child/")),
                "Should create child directory");
    assert_true(mla_fs_directory_exists(mla_string("/parent/child/")),
                "Nested directory should exist");

    // Create a nested directory without creating its parents first
    assert_true(mla_fs_create_directory(mla_string("/recursive/parent/child/")),
                "Should recursively create missing parent directories");
    assert_true(mla_fs_directory_exists(mla_string("/recursive/")),
                "Recursively created root parent should exist");
    assert_true(mla_fs_directory_exists(mla_string("/recursive/parent/")),
                "Recursively created direct parent should exist");
    assert_true(mla_fs_directory_exists(mla_string("/recursive/parent/child/")),
                "Recursively created child should exist");

    // Cleanup
    mla_fs_delete_directory(mla_string("/recursive/"));
    mla_fs_delete_directory(mla_string("/parent/child/"));
    mla_fs_delete_directory(mla_string("/parent/"));
    mla_fs_delete_directory(mla_string("/newdir/"));
}

void FileSystemDeleteDirectoryTest() {
    // Create test directory
    assert_true(mla_fs_create_directory(mla_string("/toremove/")),
                "Should create directory to delete");
    assert_true(mla_fs_directory_exists(mla_string("/toremove/")),
                "Directory should exist before deletion");

    // Delete directory
    assert_true(mla_fs_delete_directory(mla_string("/toremove/")),
                "Should successfully delete directory");
    assert_false(mla_fs_directory_exists(mla_string("/toremove/")),
                 "Directory should not exist after deletion");

    // Try deleting non-existent directory
    assert_false(mla_fs_delete_directory(mla_string("/nonexistent/")),
                 "Deleting non-existent directory should fail");

    // Delete a directory containing nested directories and files
    assert_true(mla_fs_create_directory(mla_string("/recursive-delete/child/grandchild/")),
                "Should create nested directories for recursive deletion");
    assert_true(mla_file_system_test_write_string(mla_string("/recursive-delete/root.txt"), mla_string_const("root")),
                "Should create a file in the root directory");
    assert_true(mla_file_system_test_write_string(mla_string("/recursive-delete/child/child.txt"), mla_string_const("child")),
                "Should create a file in the child directory");
    assert_true(mla_file_system_test_write_string(mla_string("/recursive-delete/child/grandchild/deep.txt"), mla_string_const("deep")),
                "Should create a file in the deepest directory");

    assert_true(mla_fs_delete_directory(mla_string("/recursive-delete/")),
                "Should recursively delete a directory and all of its contents");
    assert_false(mla_fs_directory_exists(mla_string("/recursive-delete/")),
                 "Recursively deleted root directory should not exist");
    assert_false(mla_fs_file_exists(mla_string("/recursive-delete/child/grandchild/deep.txt")),
                 "Files in recursively deleted directories should not exist");
}

void FileSystemListDirectoryTest() {
    // Create test directory structure
    assert_true(mla_fs_create_directory(mla_string("/listtest/")),
                "Should create test directory");
    assert_true(mla_fs_create_directory(mla_string("/listtest/subdir1/")),
                "Should create subdirectory 1");
    assert_true(mla_fs_create_directory(mla_string("/listtest/subdir2/")),
                "Should create subdirectory 2");

    mla_file_system_stream_t stream = mla_file_system_stream_empty();
    mla_fs_open_file(mla_string("/listtest/file1.txt"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);
    mla_fs_open_file(mla_string("/listtest/file2.txt"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);
    // Close the stream
    stream = mla_file_system_stream_empty();
    (void)stream;

    // List directory contents
    mla_array_list_t<mla_init_struct(mla_string_t)> entries = mla_array_list_empty<mla_init_struct(mla_string_t)>();
    assert_true(mla_fs_list_directory(mla_string("/listtest/"), entries),
                "Should list directory contents");

    assert_equal(mla_array_list_size(entries), (mla_size_t)2,
                 "Should have 4 entries (2 dirs)");

    // Cleanup
    mla_fs_delete_file(mla_string("/listtest/file1.txt"));
    mla_fs_delete_file(mla_string("/listtest/file2.txt"));
    mla_fs_delete_directory(mla_string("/listtest/subdir1/"));
    mla_fs_delete_directory(mla_string("/listtest/subdir2/"));
    mla_fs_delete_directory(mla_string("/listtest/"));
}

void FileSystemListFilesTest() {
    // Create test directory with files
    assert_true(mla_fs_create_directory(mla_string("/filestest/")),
                "Should create test directory");

    mla_file_system_stream_t stream = mla_file_system_stream_empty();
    mla_fs_open_file(mla_string("/filestest/doc1.txt"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);
    mla_fs_open_file(mla_string("/filestest/doc2.txt"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);
    mla_fs_open_file(mla_string("/filestest/image.png"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);
    // Close the stream
    stream = mla_file_system_stream_empty();
    (void)stream;

    // List only files
    mla_array_list_t<mla_init_struct(mla_string_t)> files = mla_array_list_empty<mla_init_struct(mla_string_t)>();
    assert_true(mla_fs_list_files(mla_string("/filestest/"), files),
                "Should list files");

    assert_equal(mla_array_list_size(files), (mla_size_t)3,
                 "Should have 3 files");

    // Cleanup
    mla_fs_delete_file(mla_string("/filestest/doc1.txt"));
    mla_fs_delete_file(mla_string("/filestest/doc2.txt"));
    mla_fs_delete_file(mla_string("/filestest/image.png"));
    mla_fs_delete_directory(mla_string("/filestest/"));
}

void FileSystemOpenFileTest() {
    assert_true(mla_fs_create_directory(mla_string("/opentest/")),
                "Should create test directory");

    // Test write mode
    mla_file_system_stream_t writeStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/opentest/write.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, writeStream),
                "Should open file in write mode");
    assert_true(mla_fs_file_exists(mla_string("/opentest/write.txt")),
                "File should exist after opening in write mode");
    // Close the stream
    writeStream = mla_file_system_stream_empty();
    (void)writeStream;

    // Test read mode
    mla_file_system_stream_t readStream = mla_file_system_stream_empty();;
    assert_true(mla_fs_open_file(mla_string("/opentest/write.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, readStream),
                "Should open existing file in read mode");
    // Close the stream
    readStream = mla_file_system_stream_empty();
    (void)readStream;

    // Test read/write mode
    mla_file_system_stream_t rwStream = mla_file_system_stream_empty();;
    assert_true(mla_fs_open_file(mla_string("/opentest/write.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE, rwStream),
                "Should open file in read/write mode");
    // Close the stream
    rwStream = mla_file_system_stream_empty();
    (void)rwStream;

    // Cleanup
    mla_fs_delete_file(mla_string("/opentest/write.txt"));
    mla_fs_delete_directory(mla_string("/opentest/"));
}

void FileSystemReadWriteDataTest() {
    // Create test directory
    assert_true(mla_fs_create_directory(mla_string("/rwtest/")),
                "Should create test directory");

    // Test data to write
    const char* testData = "Hello, this is test data for file I/O!";
    mla_size_t dataLength = mla_strlen(testData);

    // Open file for writing
    mla_file_system_stream_t writeStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/rwtest/data.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, writeStream),
                "Should open file in write mode");

    // Write test data
    if (writeStream.write != nullptr) {
        mla_size_t bytesWritten = writeStream.write(writeStream, 0, dataLength,
                                               mla_r_cast<const mla_byte_t*>(testData));
        assert_equal(bytesWritten, dataLength,
                    "Should write all test data bytes");
    } else {
        assert_true(false, "Write function should not be null");
    }


    // Close write stream
    writeStream = mla_file_system_stream_empty();

    // Verify file exists
    assert_true(mla_fs_file_exists(mla_string("/rwtest/data.txt")),
                "File should exist after writing");

    // Open file for reading
    mla_file_system_stream_t readStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/rwtest/data.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, readStream),
                "Should open file in read mode");

    // Get file length to verify
    mla_size_t fileLength = 0;
    if (readStream.length != nullptr) {
        fileLength = readStream.length(readStream);
        assert_equal(fileLength, dataLength,
                    "File length should match written data length");
    } else {
        assert_true(false, "Length function should not be null");
    }


    // Read data back
    mla_byte_t* readBuffer = mla_s_cast<mla_byte_t*>(mla_platform_malloc(fileLength + 1));

    if (readBuffer != nullptr && readStream.read != nullptr) {
        mla_size_t bytesRead = readStream.read(readStream, 0, fileLength, readBuffer);
        assert_equal(bytesRead, dataLength,
                    "Should read all data bytes");
        // Null-terminate for string comparison
        readBuffer[bytesRead] = 0;

        // Compare data
        assert_equal(mla_memcmp(readBuffer, testData, dataLength), 0,
                    "Read data should match written data");
    } else {
        assert_true(false, "Should allocate read buffer");
    }

    // Close read stream
    readStream = mla_file_system_stream_empty();

    // Clean up
    mla_platform_free(readBuffer);
    mla_fs_delete_file(mla_string("/rwtest/data.txt"));
    mla_fs_delete_directory(mla_string("/rwtest/"));
}

void FileSystemEmptyFactoriesTest() {
    mla_file_system_stream_t emptyStream = mla_file_system_stream_empty();
    mla_file_system_stream_t initStream = mla_file_system_stream_t::init();
    mla_file_system_t emptyFileSystem = mla_file_system_empty();

    assert_true(mla_string_is_empty(emptyStream.path), "Empty stream path should be empty");
    assert_true(emptyStream.seek == nullptr, "Empty stream seek should be null");
    assert_true(emptyStream.position == nullptr, "Empty stream position should be null");
    assert_true(emptyStream.length == nullptr, "Empty stream length should be null");
    assert_true(emptyStream.set_length == nullptr, "Empty stream set_length should be null");
    assert_true(emptyStream.read == nullptr, "Empty stream read should be null");
    assert_true(emptyStream.write == nullptr, "Empty stream write should be null");
    assert_true(mla_pointer_is_null(emptyStream.resource), "Empty stream resource should be null");

    assert_true(mla_string_is_empty(initStream.path), "Initialized stream path should be empty");
    assert_true(initStream.read == nullptr, "Initialized stream read should be null");
    assert_true(initStream.write == nullptr, "Initialized stream write should be null");
    assert_true(mla_pointer_is_null(initStream.resource), "Initialized stream resource should be null");

    assert_true(emptyFileSystem.file_exists == nullptr, "Empty file system file_exists should be null");
    assert_true(emptyFileSystem.open_file == nullptr, "Empty file system open_file should be null");
    assert_true(emptyFileSystem.delete_file == nullptr, "Empty file system delete_file should be null");
    assert_true(emptyFileSystem.list_files == nullptr, "Empty file system list_files should be null");
    assert_true(emptyFileSystem.create_directory == nullptr, "Empty file system create_directory should be null");
    assert_true(emptyFileSystem.directory_exists == nullptr, "Empty file system directory_exists should be null");
    assert_true(emptyFileSystem.delete_directory == nullptr, "Empty file system delete_directory should be null");
    assert_true(emptyFileSystem.list_directory == nullptr, "Empty file system list_directory should be null");
    assert_true(emptyFileSystem.os_absolute_path == nullptr, "Empty file system os_absolute_path should be null");
    assert_true(mla_pointer_is_null(emptyFileSystem.resource), "Empty file system resource should be null");
}

void FileSystemStreamWrapperTest() {
    assert_true(mla_fs_create_directory(mla_string("/streamwrap/")),
                "Should create stream wrapper test directory");

    mla_string_t writtenText = mla_string_const("stream wrapper");
    mla_size_t writtenLength = mla_string_length(writtenText);

    mla_file_system_stream_t writeStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/streamwrap/data.txt"),
                                 MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, writeStream),
                "Should open file in write mode");

    assert_equal(mla_pointer_ref_count(writeStream.resource), 1,
                 "Write-only stream should have a reference count of 1");

    mla_stream_input_t noopInput = mla_file_system_stream_as_input(writeStream);
    mla_byte_t noopReadBuffer[4] = {0};
    assert_equal(noopInput.read(noopInput, 0, sizeof(noopReadBuffer), noopReadBuffer), (mla_size_t)0,
                 "Write-only stream should map to a no-op input stream");

    mla_stream_output_t output = mla_file_system_stream_as_output(writeStream);

    assert_equal(output.write(output, 0, writtenLength,
                              mla_r_cast<const mla_byte_t*>(mla_string_data(writtenText))), writtenLength,
                 "Output wrapper should forward writes");

    noopInput = mla_stream_noop_input();
    output = mla_stream_noop_output();
    writeStream = mla_file_system_stream_empty();

    mla_file_system_stream_t readStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/streamwrap/data.txt"),
                                 MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, readStream),
                "Should open file in read mode");

    mla_stream_input_t input = mla_file_system_stream_as_input(readStream);
    assert_true(input.remaining_bytes != nullptr, "Readable stream should expose remaining bytes");

    if (input.remaining_bytes == nullptr) {
        assert_fail("Readable stream should expose remaining bytes");
    } else {
        assert_equal(input.remaining_bytes(input), writtenLength,
                     "Readable stream remaining bytes should match file length");
    }
    

    mla_byte_t readBuffer[32] = {0};
    assert_equal(input.read(input, 0, sizeof(readBuffer), readBuffer), writtenLength,
                 "Input wrapper should read the written bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(readBuffer, mla_string_data(writtenText), writtenLength),
                 (mla_test_int32_t)0, "Input wrapper should return the written content");

    mla_stream_output_t noopOutput = mla_file_system_stream_as_output(readStream);
    assert_equal(noopOutput.write(noopOutput, 0, writtenLength,
                                  mla_r_cast<const mla_byte_t*>(mla_string_data(writtenText))), writtenLength,
                 "Read-only stream should map to a no-op output stream");

    mla_fs_delete_file(mla_string("/streamwrap/data.txt"));
    mla_fs_delete_directory(mla_string("/streamwrap/"));
}

void FileSystemLifecycleTest() {
    mla_bool_t wasLocked = mla_file_system_is_locked();

    if (wasLocked) {
        mla_file_system_unlock();
    }

    mla_bool_t unlocked = !mla_file_system_is_locked();
    mla_bool_t invalidMountRejected = !mla_file_system_initialize(mla_string("invalid"), mla_file_system_empty());
    mla_bool_t mounted = mla_file_system_initialize(mla_string("/apitest/"), mla_file_system_empty());
    mla_bool_t removed = mla_file_system_deinitialize(mla_string("/APITEST/"));
    mla_bool_t missingMountRejected = !mla_file_system_deinitialize(mla_string("/apitest/"));

    if (mounted && !removed) {
        (void)mla_file_system_deinitialize(mla_string("/apitest/"));
    }

    mla_file_system_lock();
    mla_bool_t locked = mla_file_system_is_locked();
    mla_bool_t lockedMountRejected = !mla_file_system_initialize(mla_string("/locked/"), mla_file_system_empty());

    if (!wasLocked) {
        mla_file_system_unlock();
    }

    assert_true(unlocked, "File system should unlock");
    assert_true(invalidMountRejected, "Invalid mount path should be rejected");
    assert_true(mounted, "Valid mount path should be accepted while unlocked");
    assert_true(removed, "Mounted file system should be removable");
    assert_true(missingMountRejected, "Removing a missing mount should fail");
    assert_true(locked, "File system should lock");
    assert_true(lockedMountRejected, "Mounting while locked should fail");
    assert_equal(mla_file_system_is_locked(), wasLocked, "Lock state should be restored");
}

void FileSystemCountFilesTest() {
    assert_true(mla_fs_create_directory(mla_string("/countfiles/")),
                "Should create count files directory");
    assert_true(mla_fs_create_directory(mla_string("/countfiles/subdir/")),
                "Should create nested directory");
    assert_true(mla_file_system_test_write_string(mla_string("/countfiles/one.txt"), mla_string_const("1")),
                "Should create first file");
    assert_true(mla_file_system_test_write_string(mla_string("/countfiles/two.txt"), mla_string_const("22")),
                "Should create second file");

    mla_size_t count = 0;
    assert_true(mla_fs_count_files(mla_string("/countfiles/"), count),
                "Should count files in directory");
    assert_equal(count, (mla_size_t)2, "Should count only files");

    mla_fs_delete_file(mla_string("/countfiles/one.txt"));
    mla_fs_delete_file(mla_string("/countfiles/two.txt"));
    mla_fs_delete_directory(mla_string("/countfiles/subdir/"));
    mla_fs_delete_directory(mla_string("/countfiles/"));
}

void FileSystemCountDirectoryTest() {
    assert_true(mla_fs_create_directory(mla_string("/countdirs/")),
                "Should create count directories root");
    assert_true(mla_fs_create_directory(mla_string("/countdirs/first/")),
                "Should create first child directory");
    assert_true(mla_fs_create_directory(mla_string("/countdirs/second/")),
                "Should create second child directory");
    assert_true(mla_file_system_test_write_string(mla_string("/countdirs/file.txt"), mla_string_const("content")),
                "Should create sibling file");

    mla_size_t count = 0;
    assert_true(mla_fs_count_directory(mla_string("/countdirs/"), count),
                "Should count directories");
    assert_equal(count, (mla_size_t)2, "Should count only directories");

    mla_fs_delete_file(mla_string("/countdirs/file.txt"));
    mla_fs_delete_directory(mla_string("/countdirs/first/"));
    mla_fs_delete_directory(mla_string("/countdirs/second/"));
    mla_fs_delete_directory(mla_string("/countdirs/"));
}

void FileSystemCopyApisTest() {
    mla_string_t sourceContent = mla_string_const("copy me");

    assert_true(mla_fs_create_directory(mla_string("/copytest/")),
                "Should create copy test directory");
    assert_true(mla_file_system_test_write_string(mla_string("/copytest/source.txt"), sourceContent),
                "Should create source file");

    assert_true(mla_fs_copy_file_to(mla_string("/copytest/source.txt"), mla_string("/copytest/destination.txt")),
                "Should copy file to file");
    assert_true(mla_string_equals(mla_file_system_test_read_string(mla_string("/copytest/destination.txt")), sourceContent),
                "Copied destination file should match source content");

    mla_memory_stream_t copiedToStream = mla_memory_stream_empty();
    assert_true(mla_fs_copy_file_to_stream(mla_string("/copytest/source.txt"), copiedToStream.output),
                "Should copy file to output stream");
    assert_true(mla_memory_stream_set_position(copiedToStream, 0), "Should rewind copied stream");
    assert_true(mla_string_equals(
                    mla_string_from_stream(copiedToStream.input, mla_memory_stream_get_size(copiedToStream) + 1),
                    sourceContent),
                "Copied stream content should match source content");

    mla_stream_input_t sourceStream = mla_stream_input_from_string(mla_string_const("stream copy"));
    assert_true(mla_fs_copy_stream_to_file(sourceStream, mla_string("/copytest/from-stream.txt")),
                "Should copy input stream to file");
    assert_true(mla_string_equals(mla_file_system_test_read_string(mla_string("/copytest/from-stream.txt")),
                                  mla_string_const("stream copy")),
                "Copied file should match stream content");

    mla_fs_delete_file(mla_string("/copytest/source.txt"));
    mla_fs_delete_file(mla_string("/copytest/destination.txt"));
    mla_fs_delete_file(mla_string("/copytest/from-stream.txt"));
    mla_fs_delete_directory(mla_string("/copytest/"));
}

void FileSystemCopyDirectoryTest() {
    assert_true(mla_fs_create_directory(mla_string("/copy-directory/source/empty/")),
                "Should create the source directory tree");
    assert_true(mla_fs_create_directory(mla_string("/copy-directory/source/nested/deep/")),
                "Should create nested source directories");
    assert_true(mla_file_system_test_write_string(
                    mla_string("/copy-directory/source/root.txt"), mla_string_const("root")),
                "Should create the source root file");
    assert_true(mla_file_system_test_write_string(
                    mla_string("/copy-directory/source/nested/deep/file.txt"), mla_string_const("deep")),
                "Should create the nested source file");

    assert_true(mla_fs_copy_directory(mla_string("/copy-directory/source/"),
                                      mla_string("/copy-directory/destination/")),
                "Should recursively copy a directory");
    assert_true(mla_fs_directory_exists(mla_string("/copy-directory/destination/empty/")),
                "Should copy empty directories");
    assert_true(mla_string_equals(
                    mla_file_system_test_read_string(mla_string("/copy-directory/destination/root.txt")),
                    mla_string_const("root")),
                "Should copy files from the source root");
    assert_true(mla_string_equals(
                    mla_file_system_test_read_string(
                        mla_string("/copy-directory/destination/nested/deep/file.txt")),
                    mla_string_const("deep")),
                "Should copy files from nested directories");

    assert_false(mla_fs_copy_directory(mla_string("/copy-directory/missing/"),
                                       mla_string("/copy-directory/missing-copy/")),
                 "Copying a missing directory should fail");
    assert_false(mla_fs_copy_directory(mla_string("/copy-directory/source/"),
                                       mla_string("/copy-directory/source/")),
                 "Copying a directory onto itself should fail");
    assert_false(mla_fs_copy_directory(mla_string("/copy-directory/source/"),
                                       mla_string("/copy-directory/source/child-copy/")),
                 "Copying a directory into its descendant should fail");

    assert_true(mla_fs_delete_directory(mla_string("/copy-directory/")),
                "Should clean up copied directory trees");
}

void FileSystemRelativePathTest() {
    mla_string_t childPath = mla_fs_get_relative_path(mla_string("/root/base/"),
                                                      mla_string("/root/base/file.txt"), false);
    assert_true(mla_string_equals(childPath, mla_string("/file.txt")),
                "Direct child relative path should keep the leading slash");

    mla_string_t siblingPath = mla_fs_get_relative_path(mla_string("/root/base/"),
                                                        mla_string("/root/other/file.txt"), true);
    assert_true(mla_string_equals(siblingPath, mla_string("/../other/file.txt")),
                "Relative path should navigate up when requested");

    mla_string_t noMatch = mla_fs_get_relative_path(mla_string("/root/base/"),
                                                    mla_string("/other/file.txt"), false);
    assert_true(mla_string_is_empty(noMatch), "Unrelated paths should return empty without upward navigation");
}

void FileSystemAbsolutePathTest() {
    assert_true(mla_fs_create_directory(mla_string("/abstest/")),
                "Should create absolute path test directory");
    assert_true(mla_file_system_test_write_string(mla_string("/abstest/file.txt"), mla_string_const("absolute")),
                "Should create file for absolute path test");

    mla_string_t existingPath = mla_fs_get_complete_os_absolute_path(mla_string("/abstest/file.txt"));
    assert_false(mla_string_is_empty(existingPath), "Existing file should resolve to an OS path");

    assert_true(mla_string_ends_with(existingPath, mla_string("file.txt")),
                "Resolved path should end with the original file path");

    mla_string_t uncheckedPath = mla_fs_get_complete_os_absolute_path(mla_string("/abstest/missing.txt"), false);
    assert_false(mla_string_is_empty(uncheckedPath), "Unchecked missing file should still resolve");
    assert_true(mla_string_ends_with(uncheckedPath, mla_string("missing.txt")),
                "Unchecked resolved path should end with the missing file path");

    mla_fs_delete_file(mla_string("/abstest/file.txt"));
    mla_fs_delete_directory(mla_string("/abstest/"));
}

void RegisterFileSystemPathTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("IsDirectoryPath", test_category, FileSystemIsDirectoryPathTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetParentDirectory", test_category, FileSystemGetParentDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetFileName", test_category, FileSystemGetFileNameTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetFileExtension", test_category, FileSystemGetFileExtensionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ChangeFileExtension", test_category, FileSystemChangeFileExtensionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CombinePaths", test_category, FileSystemCombinePathsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FileExists", test_category, FileSystemFileExistsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DeleteFile", test_category, FileSystemDeleteFileTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DirectoryExists", test_category, FileSystemDirectoryExistsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CreateDirectory", test_category, FileSystemCreateDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DeleteDirectory", test_category, FileSystemDeleteDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ListDirectory", test_category, FileSystemListDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ListFiles", test_category, FileSystemListFilesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("OpenFile", test_category, FileSystemOpenFileTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ReadWriteData", test_category, FileSystemReadWriteDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EmptyFactories", test_category, FileSystemEmptyFactoriesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamWrapper", test_category, FileSystemStreamWrapperTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Lifecycle", test_category, FileSystemLifecycleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CountFiles", test_category, FileSystemCountFilesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CountDirectory", test_category, FileSystemCountDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CopyApis", test_category, FileSystemCopyApisTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CopyDirectory", test_category, FileSystemCopyDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RelativePath", test_category, FileSystemRelativePathTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AbsolutePath", test_category, FileSystemAbsolutePathTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}


#endif
