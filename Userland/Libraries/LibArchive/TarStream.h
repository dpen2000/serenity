/*
 * Copyright (c) 2020, Peter Elliott <pelliott@serenityos.org>
 * Copyright (c) 2021, Idan Horowitz <idan.horowitz@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Span.h>
#include <LibArchive/Tar.h>
#include <LibCore/Stream.h>

namespace Archive {

class TarInputStream;

class TarFileStream : public Core::Stream::Stream {
public:
    virtual ErrorOr<Bytes> read(Bytes) override;
    virtual ErrorOr<size_t> write(ReadonlyBytes) override;
    virtual bool is_eof() const override;
    virtual bool is_open() const override { return true; };
    virtual void close() override {};

private:
    TarFileStream(TarInputStream& stream);
    TarInputStream& m_tar_stream;
    int m_generation;

    friend class TarInputStream;
};

class TarInputStream {
public:
    static ErrorOr<NonnullOwnPtr<TarInputStream>> construct(NonnullOwnPtr<Core::Stream::Stream>);
    ErrorOr<void> advance();
    bool finished() const { return m_found_end_of_archive || m_stream->is_eof(); }
    ErrorOr<bool> valid() const;
    TarFileHeader const& header() const { return m_header; }
    TarFileStream file_contents();

    template<VoidFunction<StringView, StringView> F>
    ErrorOr<void> for_each_extended_header(F func);

private:
    TarInputStream(NonnullOwnPtr<Core::Stream::Stream>);
    ErrorOr<void> load_next_header();

    TarFileHeader m_header;
    NonnullOwnPtr<Core::Stream::Stream> m_stream;
    unsigned long m_file_offset { 0 };
    int m_generation { 0 };
    bool m_found_end_of_archive { false };

    friend class TarFileStream;
};

class TarOutputStream {
public:
    TarOutputStream(Core::Stream::Handle<Core::Stream::Stream>);
    ErrorOr<void> add_file(DeprecatedString const& path, mode_t, ReadonlyBytes);
    ErrorOr<void> add_link(DeprecatedString const& path, mode_t, StringView);
    ErrorOr<void> add_directory(DeprecatedString const& path, mode_t);
    ErrorOr<void> finish();

private:
    Core::Stream::Handle<Core::Stream::Stream> m_stream;
    bool m_finished { false };

    friend class TarFileStream;
};

template<VoidFunction<StringView, StringView> F>
inline ErrorOr<void> TarInputStream::for_each_extended_header(F func)
{
    VERIFY(header().content_is_like_extended_header());

    Archive::TarFileStream file_stream = file_contents();

    auto header_size = TRY(header().size());
    ByteBuffer file_contents_buffer = TRY(ByteBuffer::create_zeroed(header_size));
    VERIFY(TRY(file_stream.read(file_contents_buffer)).size() == header_size);

    StringView file_contents { file_contents_buffer };

    while (!file_contents.is_empty()) {
        // Split off the length (until the first space).
        Optional<size_t> length_end_index = file_contents.find(' ');
        if (!length_end_index.has_value())
            return Error::from_string_literal("Malformed extended header: No length found.");
        Optional<unsigned int> length = file_contents.substring_view(0, length_end_index.value()).to_uint();
        if (!length.has_value())
            return Error::from_string_literal("Malformed extended header: Could not parse length.");
        unsigned int remaining_length = length.value();

        remaining_length -= length_end_index.value() + 1;
        file_contents = file_contents.substring_view(length_end_index.value() + 1);

        // Extract the header.
        StringView header = file_contents.substring_view(0, remaining_length - 1);
        file_contents = file_contents.substring_view(remaining_length - 1);

        // Ensure that the header ends at the expected location.
        if (file_contents.length() < 1 || !file_contents.starts_with('\n'))
            return Error::from_string_literal("Malformed extended header: Header does not end at expected location.");
        file_contents = file_contents.substring_view(1);

        // Find the delimiting '='.
        Optional<size_t> header_delimiter_index = header.find('=');
        if (!header_delimiter_index.has_value())
            return Error::from_string_literal("Malformed extended header: Header does not have a delimiter.");
        StringView key = header.substring_view(0, header_delimiter_index.value());
        StringView value = header.substring_view(header_delimiter_index.value() + 1);

        func(key, value);
    }

    return {};
}

}
