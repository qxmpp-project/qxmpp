// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppFutureUtils_p.h"
#include "QXmppHash.h"
#include "QXmppHashing_p.h"

#include <QCryptographicHash>
#include <QFuture>
#include <QFutureInterface>
#include <QIODevice>
#include <QRunnable>
#include <QThreadPool>

using namespace QXmpp;
using namespace QXmpp::Private;

class HashGenerator;

// 8 kB
constexpr std::size_t PROCESS_SYNC_MAX_SIZE = 32 * 1024;
// 512 kB (two buffers are used so 1 MB)
constexpr std::size_t BUFFER_SIZE = 512 * 1024;

/// \cond
static HashAlgorithm toHashAlgorithm(QCryptographicHash::Algorithm algorithm)
{
    switch (algorithm) {
    case QCryptographicHash::Md4:
    case QCryptographicHash::Keccak_224:
    case QCryptographicHash::Keccak_256:
    case QCryptographicHash::Keccak_384:
    case QCryptographicHash::Keccak_512:
    case QCryptographicHash::Sha3_224:
    case QCryptographicHash::Sha3_384:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QCryptographicHash::Blake2b_160:
    case QCryptographicHash::Blake2b_384:
    case QCryptographicHash::Blake2s_128:
    case QCryptographicHash::Blake2s_160:
    case QCryptographicHash::Blake2s_224:
    case QCryptographicHash::Blake2s_256:
#endif
        return HashAlgorithm::Unknown;
    case QCryptographicHash::Md5:
        return HashAlgorithm::Md5;
    case QCryptographicHash::Sha1:
        return HashAlgorithm::Sha1;
    case QCryptographicHash::Sha224:
        return HashAlgorithm::Sha224;
    case QCryptographicHash::Sha256:
        return HashAlgorithm::Sha256;
    case QCryptographicHash::Sha384:
        return HashAlgorithm::Sha384;
    case QCryptographicHash::Sha512:
        return HashAlgorithm::Sha512;
    case QCryptographicHash::Sha3_256:
        return HashAlgorithm::Sha3_256;
    case QCryptographicHash::Sha3_512:
        return HashAlgorithm::Sha3_512;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QCryptographicHash::Blake2b_256:
        return HashAlgorithm::Blake2b_256;
    case QCryptographicHash::Blake2b_512:
        return HashAlgorithm::Blake2b_512;
#endif
    }
    return HashAlgorithm::Unknown;
}

static std::optional<QCryptographicHash::Algorithm> toCryptograhicHashAlgorithm(HashAlgorithm algorithm)
{
    switch (algorithm) {
    case HashAlgorithm::Unknown:
    case HashAlgorithm::Md2:
    case HashAlgorithm::Shake128:
    case HashAlgorithm::Shake256:
        return {};
    case HashAlgorithm::Md5:
        return QCryptographicHash::Md5;
    case HashAlgorithm::Sha1:
        return QCryptographicHash::Sha1;
    case HashAlgorithm::Sha224:
        return QCryptographicHash::Sha224;
    case HashAlgorithm::Sha256:
        return QCryptographicHash::Sha256;
    case HashAlgorithm::Sha384:
        return QCryptographicHash::Sha384;
    case HashAlgorithm::Sha512:
        return QCryptographicHash::Sha512;
    case HashAlgorithm::Sha3_256:
        return QCryptographicHash::Sha3_256;
    case HashAlgorithm::Sha3_512:
        return QCryptographicHash::Sha3_512;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case HashAlgorithm::Blake2b_256:
        return QCryptographicHash::Blake2b_256;
    case HashAlgorithm::Blake2b_512:
        return QCryptographicHash::Blake2b_512;
#else
    case HashAlgorithm::Blake2b_256:
    case HashAlgorithm::Blake2b_512:
        return {};
#endif
    }
    return {};
}

bool QXmpp::Private::isHashingAlgorithmSecure(HashAlgorithm algorithm)
{
    switch (algorithm) {
    case HashAlgorithm::Unknown:
    case HashAlgorithm::Md2:
    case HashAlgorithm::Md5:
    case HashAlgorithm::Shake128:
    case HashAlgorithm::Shake256:
    case HashAlgorithm::Sha1:
        return false;
    case HashAlgorithm::Sha224:
    case HashAlgorithm::Sha256:
    case HashAlgorithm::Sha384:
    case HashAlgorithm::Sha512:
    case HashAlgorithm::Sha3_256:
    case HashAlgorithm::Sha3_512:
    case HashAlgorithm::Blake2b_256:
    case HashAlgorithm::Blake2b_512:
        return true;
    }
    return false;
}

uint16_t QXmpp::Private::hashPriority(HashAlgorithm algorithm)
{
    switch (algorithm) {
    case HashAlgorithm::Unknown:
        return 0;
    case HashAlgorithm::Md2:
        return 1;
    case HashAlgorithm::Md5:
        return 2;
    case HashAlgorithm::Shake128:
        return 3;
    case HashAlgorithm::Shake256:
        return 4;
    case HashAlgorithm::Sha1:
        return 5;
    case HashAlgorithm::Sha224:
        return 6;
    case HashAlgorithm::Sha256:
        return 7;
    case HashAlgorithm::Sha384:
        return 8;
    case HashAlgorithm::Sha512:
        return 9;
    // prefer BLAKE2 over SHA3 because BLAKE2 is faster
    // prefer 512 bits over 256 bits
    case HashAlgorithm::Sha3_256:
        return 10;
    case HashAlgorithm::Blake2b_256:
        return 11;
    case HashAlgorithm::Sha3_512:
        return 12;
    case HashAlgorithm::Blake2b_512:
        return 13;
    }
    return 0;
}

template<typename T, typename Converter>
auto transform(std::vector<T> &input, Converter convert)
{
    using Output = std::decay_t<decltype(convert(input.front()))>;
    std::vector<Output> output;
    output.reserve(input.size());
    std::transform(input.begin(), input.end(), std::back_inserter(output), std::move(convert));
    return output;
}

auto makeReadyResult(HashingResult::Result result, std::unique_ptr<QIODevice> device)
{
    return makeReadyFuture<HashingResultPtr>(std::make_shared<HashingResult>(std::move(result), std::move(device)));
}

auto makeReadyResult(HashVerificationResult::Result result, std::unique_ptr<QIODevice> data)
{
    return makeReadyFuture<HashVerificationResultPtr>(std::make_shared<HashVerificationResult>(result, std::move(data)));
}

auto deviceSize(QIODevice &device) -> std::optional<std::size_t>
{
    if (!device.isSequential()) {
        if (auto size = device.size(); size >= 0) {
            return size;
        }
    }
    return std::nullopt;
}

HashingResult calculateHashesSync(std::unique_ptr<QIODevice> data, std::vector<QCryptographicHash::Algorithm> algorithms)
{
    std::vector<QXmppHash> results;
    results.reserve(algorithms.size());
    for (auto algorithm : algorithms) {
        QCryptographicHash hasher(algorithm);
        data->seek(0);
        if (!hasher.addData(data.get())) {
            return { QXmppError::fromIoDevice(*data), std::move(data) };
        }

        QXmppHash hash;
        hash.setAlgorithm(toHashAlgorithm(algorithm));
        hash.setHash(hasher.result());
        results.push_back(hash);
    }
    return { std::move(results), std::move(data) };
}

struct BufferReader : public QRunnable
{
    BufferReader(HashGenerator &creator)
        : generator(creator)
    {
        setAutoDelete(false);
    }
    ~BufferReader() override = default;

    void run() override;

    HashGenerator &generator;
};

struct HashProcessor : public QRunnable
{
    HashProcessor(HashGenerator *generator, QCryptographicHash::Algorithm algorithm)
        : generator(generator),
          hash(std::make_unique<QCryptographicHash>(algorithm)),
          algorithm(algorithm)
    {
        setAutoDelete(false);
    }
    HashProcessor(HashProcessor &&other) noexcept
        : generator(other.generator),
          hash(std::move(other.hash)),
          algorithm(other.algorithm)
    {
    }
    ~HashProcessor() override = default;

    void run() override;

    HashGenerator *generator;
    std::unique_ptr<QCryptographicHash> hash;
    QCryptographicHash::Algorithm algorithm;
};

class HashGenerator : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HashGenerator)
public:
    static void calculateHashes(std::unique_ptr<QIODevice> data,
                                std::vector<HashAlgorithm> algorithms,
                                std::function<void(HashingResult)> reportResult,
                                std::function<bool()> isCancelled)
    {
        // convert to QCryptographicHash::Algorithm for hashing
        auto qtAlgorithms = transform(algorithms, [](auto algorithm) {
            auto converted = toCryptograhicHashAlgorithm(algorithm);
            Q_ASSERT_X(converted.has_value(), "calculate hashes", "Must only be called with algorithms supported by QCryptographicHash");
            return *converted;
        });

        // check for readability
        if (!data->isOpen() || !data->isReadable()) {
            reportResult({ QXmppError {
                               QStringLiteral("Input data is not opened for reading."),
                               std::any() },
                           std::move(data) });
            return;
        }
        data->setParent(nullptr);
        // We don't want to move data to the right thread each time and we don't need the event
        // processing here anyways.
        data->moveToThread(nullptr);

        // optimization for small data
        if (auto size = deviceSize(*data)) {
            if ((algorithms.size() * data->size()) <= PROCESS_SYNC_MAX_SIZE) {
                reportResult(calculateHashesSync(std::move(data), std::move(qtAlgorithms)));
                return;
            }
        }

        // start normal hash calculation with hash generator
        new HashGenerator(std::move(data), std::move(qtAlgorithms), std::move(reportResult), std::move(isCancelled));
    }

    HashGenerator(std::unique_ptr<QIODevice> data,
                  std::vector<QCryptographicHash::Algorithm> algorithms,
                  std::function<void(HashingResult)> reportResult,
                  std::function<bool()> isCancelled)
        : m_data(std::move(data)),
          m_bufferReader(*this),
          m_reportResult(std::move(reportResult)),
          m_isCancelled(std::move(isCancelled))
    {
        // create hash processors
        m_hashProcessors = transform(algorithms, [this](auto algorithm) {
            return HashProcessor(this, algorithm);
        });

        // create buffers
        auto size = deviceSize(*m_data);
        if (size && *size <= 2 * BUFFER_SIZE) {
            // read everything in one go
            m_readBuffer.reserve(*size);
        } else {
            m_readBuffer.reserve(BUFFER_SIZE);
            m_processBuffer.reserve(BUFFER_SIZE);
        }

        // start reading buffer
        m_runningJobs = 1;
        QThreadPool::globalInstance()->start(&m_bufferReader);
    }
    ~HashGenerator() override = default;

    void startNextIteration()
    {
        if (m_errorOccurred) {
            // error has been reported already
            deleteLater();
            return;
        }

        // reading was already finished, processing of the last data is now also finished
        if (m_readingFinished) {
            finish();
            deleteLater();
            return;
        }

        // check for cancellation
        if (m_isCancelled()) {
            m_reportResult({ Cancelled(), std::move(m_data) });
            deleteLater();
            return;
        }

        m_readingFinished = m_data->atEnd();

        // swap buffers: read data is now processed, the buffer of the
        // processed data is reused as new read buffer
        m_processBuffer.swap(m_readBuffer);

        // reset counter
        if (m_readingFinished) {
            m_runningJobs = int(m_hashProcessors.size());
        } else {
            m_runningJobs = int(m_hashProcessors.size() + 1);
        }

        auto *pool = QThreadPool::globalInstance();
        // optimization: don't restart the buffer reader if we know no more bytes can be read
        if (!m_readingFinished) {
            pool->start(&m_bufferReader);
        }
        // start all hash processors
        for (auto &hashProcessor : m_hashProcessors) {
            hashProcessor.setAutoDelete(false);
            pool->start(&hashProcessor);
        }
    }

    void reportJobFinished()
    {
        if (!m_runningJobs.deref()) {
            // no other jobs are running anymore
            // all hashes have processed the current buffer
            // a new buffer has been read
            startNextIteration();
        }
    }

    void reportBufferReadError(QXmppError err)
    {
        m_errorOccurred = true;
        m_reportResult({ std::move(err), std::move(m_data) });
    }

    void finish()
    {
        auto hashes = transform(m_hashProcessors, [](auto &processor) {
            QXmppHash hash;
            hash.setAlgorithm(toHashAlgorithm(processor.algorithm));
            hash.setHash(processor.hash->result());
            return hash;
        });
        m_reportResult({ std::move(hashes), std::move(m_data) });
    }

    bool m_errorOccurred = false;
    bool m_readingFinished = false;
    std::unique_ptr<QIODevice> m_data;
    std::vector<char> m_readBuffer;
    std::vector<char> m_processBuffer;
    QAtomicInt m_runningJobs = 0;
    std::vector<HashProcessor> m_hashProcessors;
    BufferReader m_bufferReader;
    std::function<void(HashingResult)> m_reportResult;
    std::function<bool()> m_isCancelled;
};

void BufferReader::run()
{
    auto &data = *generator.m_data;
    auto &buffer = generator.m_readBuffer;

    buffer.resize(buffer.capacity());
    auto readBytes = data.read(buffer.data(), buffer.size());

    // negative values indicate errors
    if (readBytes < 0) {
        buffer.resize(0);
        generator.reportBufferReadError(QXmppError::fromIoDevice(*generator.m_data));
    } else {
        buffer.resize(readBytes);
    }
    generator.reportJobFinished();
}

void HashProcessor::run()
{
    auto &buffer = generator->m_processBuffer;
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    hash->addData(QByteArrayView(buffer.data(), buffer.size()));
#else
    hash->addData(buffer.data(), buffer.size());
#endif
    generator->reportJobFinished();
}

QFuture<HashingResultPtr> QXmpp::Private::calculateHashes(std::unique_ptr<QIODevice> data, std::vector<HashAlgorithm> algorithms)
{
    QFutureInterface<HashingResultPtr> interface;
    auto finish = [interface](HashingResult &&result) mutable {
        interface.reportResult(std::make_shared<HashingResult>(std::move(result)));
        interface.reportFinished();
    };
    auto isCancelled = [interface]() mutable {
        return interface.isCanceled();
    };

    // object will delete itself using QObject::deleteLater()
    HashGenerator::calculateHashes(std::move(data), std::move(algorithms), std::move(finish), std::move(isCancelled));
    return interface.future();
}

QFuture<HashVerificationResultPtr> QXmpp::Private::verifyHashes(std::unique_ptr<QIODevice> data, std::vector<QXmppHash> hashes)
{
    // filter out invalid hashes and insecure
    auto isInvalid = [](const auto &hash) {
        return hash.hash().isEmpty() || !isHashingAlgorithmSecure(hash.algorithm());
    };
    hashes.erase(std::remove_if(hashes.begin(), hashes.end(), isInvalid), hashes.end());

    if (hashes.empty()) {
        return makeReadyResult(HashVerificationResult::NoStrongHashes(), std::move(data));
    }

    std::sort(hashes.begin(), hashes.end(), [](const auto &a, const auto &b) {
        return hashPriority(a.algorithm()) < hashPriority(b.algorithm());
    });

    auto expected = hashes.back();

    auto verifyResult = [](auto &result, auto &expected) -> HashVerificationResult::Result {
        if (auto actualHashes = std::get_if<std::vector<QXmppHash>>(&result)) {
            Q_ASSERT(!actualHashes->empty());
            if (actualHashes->front().hash() == expected.hash()) {
                return HashVerificationResult::Verified();
            }
            return HashVerificationResult::NotMatching();
        } else if (std::holds_alternative<Cancelled>(result)) {
            return Cancelled();
        }
        return std::get<QXmppError>(std::move(result));
    };

    QFutureInterface<HashVerificationResultPtr> interface;
    auto finish = [interface, expected, verifyResult](HashingResult &&hashingResult) mutable {
        auto &[result, data] = hashingResult;

        interface.reportResult(std::make_shared<HashVerificationResult>(verifyResult(result, expected), std::move(data)));
        interface.reportFinished();
    };
    auto isCancelled = [interface]() {
        return interface.isCanceled();
    };

    HashGenerator::calculateHashes(std::move(data), { expected.algorithm() }, std::move(finish), std::move(isCancelled));
    return interface.future();
}
/// \endcond

#include "QXmppHashing.moc"
