// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef OMEMOLIBWRAPPERS_H
#define OMEMOLIBWRAPPERS_H

#include <key_helper.h>
#include <session_builder.h>
#include <session_cipher.h>
#include <signal_protocol.h>

// Wraps various types of the OMEMO library.
template<typename T, void(destruct)(T *)>
class OmemoLibPtr
{
    T *m_ptr = nullptr;

public:
    OmemoLibPtr(T *ptr = nullptr) : m_ptr(ptr) { }
    OmemoLibPtr(const OmemoLibPtr &) = delete;
    ~OmemoLibPtr()
    {
        if (m_ptr) {
            destruct(m_ptr);
        }
    }
    OmemoLibPtr &operator=(const OmemoLibPtr &) = delete;
    OmemoLibPtr<T, destruct> &operator=(T *ptr)
    {
        reset(ptr);
        return *this;
    }
    operator bool() const { return m_ptr != nullptr; }
    T *operator->() const { return m_ptr; }
    T *get() const { return m_ptr; }
    T **ptrRef() { return &m_ptr; }
    void reset(T *ptr)
    {
        if (m_ptr) {
            destruct(m_ptr);
        }
        m_ptr = ptr;
    }
};

template<typename T>
void omemoLibUnrefHelper(T *ptr)
{
    SIGNAL_UNREF(ptr);
}

template<typename T>
using RefCountedPtr = OmemoLibPtr<T, omemoLibUnrefHelper<T>>;

static QByteArray omemoLibBufferToByteArray(signal_buffer *buffer)
{
    return QByteArray(reinterpret_cast<const char *>(signal_buffer_data(buffer)), signal_buffer_len(buffer));
}

static signal_buffer *omemoLibBufferFromByteArray(const QByteArray &bytes)
{
    return signal_buffer_create(reinterpret_cast<const unsigned char *>(bytes.constData()), bytes.size());
}

template<void(destruct)(signal_buffer *)>
class BufferPtrBase : public OmemoLibPtr<signal_buffer, destruct>
{
public:
    QByteArray toByteArray() const
    {
        return omemoLibBufferToByteArray(this->get());
    }
};

class BufferSecurePtr : public BufferPtrBase<signal_buffer_bzero_free>
{
public:
    static BufferSecurePtr fromByteArray(const QByteArray &bytes)
    {
        return { omemoLibBufferFromByteArray(bytes) };
    }
};

class BufferPtr : public BufferPtrBase<signal_buffer_free>
{
public:
    static BufferPtr fromByteArray(const QByteArray &bytes)
    {
        return { omemoLibBufferFromByteArray(bytes) };
    }
};

using KeyListNodePtr = OmemoLibPtr<signal_protocol_key_helper_pre_key_list_node, signal_protocol_key_helper_key_list_free>;
using SessionCipherPtr = OmemoLibPtr<session_cipher, session_cipher_free>;
using SessionBuilderPtr = OmemoLibPtr<session_builder, session_builder_free>;
using OmemoContextPtr = OmemoLibPtr<signal_context, signal_context_destroy>;
using StoreContextPtr = OmemoLibPtr<signal_protocol_store_context, signal_protocol_store_context_destroy>;

#endif  // OMEMOLIBWRAPPERS_H
