#include <omnicore/test/utils_tx.h>

#include <omnicore/createpayload.h>
#include <omnicore/encoding.h>
#include <omnicore/omnicore.h>
#include <omnicore/parsing.h>
#include <omnicore/rules.h>
#include <omnicore/script.h>
#include <omnicore/tx.h>

#include <base58.h>
#include <coins.h>
#include <key_io.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <script/standard.h>
#include <test/test_bitcoin.h>

#include <stdint.h>
#include <limits>
#include <vector>

#include <boost/test/unit_test.hpp>

using namespace mastercore;

BOOST_FIXTURE_TEST_SUITE(omnicore_parsing_a_tests, BasicTestingSetup)

/** Creates a dummy transaction with the given inputs and outputs. */
static CTransaction TxClassA(const std::vector<CTxOut>& txInputs, const std::vector<CTxOut>& txOuts)
{
    CMutableTransaction mutableTx;

    // Inputs:
    for (const auto& txOut : txInputs)
    {
        // Create transaction for input:
        CMutableTransaction inputTx;
        inputTx.vout.push_back(txOut);
        CTransaction tx(inputTx);

        // Populate transaction cache:
        Coin newcoin;
        newcoin.out.scriptPubKey = txOut.scriptPubKey;
        newcoin.out.nValue = txOut.nValue;
        view.AddCoin(COutPoint(tx.GetHash(), 0), std::move(newcoin), true);

        // Add input:
        CTxIn txIn(tx.GetHash(), 0);
        mutableTx.vin.push_back(txIn);
    }

    for (std::vector<CTxOut>::const_iterator it = txOuts.begin(); it != txOuts.end(); ++it)
    {
        const CTxOut& txOut = *it;
        mutableTx.vout.push_back(txOut);
    }

    return CTransaction(mutableTx);
}

/** Helper to create a CTxOut object. */
static CTxOut createTxOut(int64_t amount, const std::string& dest)
{
    return CTxOut(amount, GetScriptForDestination(DecodeDestination(dest)));
}

BOOST_AUTO_TEST_CASE(valid_class_a)
{
    {
        int nBlock = 0;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(1765000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));
        txInputs.push_back(createTxOut(50000, "Bv7iwfpnoTTDY7tA3xj6wQmrmdQJAT35V5"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(6000, "C4cWj6wnh7GhSTKJJVh5JtBkvCFKdEsdUm"));
        txOutputs.push_back(createTxOut(6000, "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ"));
        txOutputs.push_back(createTxOut(1747000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK_EQUAL(ParseTransaction(dummyTx, nBlock, 1, metaTx), 0);
        BOOST_CHECK_EQUAL(metaTx.getFeePaid(), 50000);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "000000000000000100000002540be400000000");
    }
    {
        int nBlock = 0;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(907500, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));
        txInputs.push_back(createTxOut(907500, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "C4cWj6wnh7GhSaUhZfbxRFKnW9F7Zvf7v2"));
        txOutputs.push_back(createTxOut(6000, "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ"));
        txOutputs.push_back(createTxOut(1747000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getFeePaid(), 50000);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "000000000000000200000002540be400000000");
    }
    {
        int nBlock = std::numeric_limits<int>::max();

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(1815000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(NonStandardOutput());
        txOutputs.push_back(NonStandardOutput());
        txOutputs.push_back(NonStandardOutput());
        txOutputs.push_back(NonStandardOutput());
        txOutputs.push_back(NonStandardOutput());
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(PayToPubKey_Unrelated());
        txOutputs.push_back(PayToPubKey_Unrelated());
        txOutputs.push_back(PayToPubKey_Unrelated());
        txOutputs.push_back(createTxOut(6000, "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ"));
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(OpReturn_Unrelated());
        txOutputs.push_back(OpReturn_Unrelated());
        txOutputs.push_back(createTxOut(6000, "C4cWj6wnh7GhSTKJJVh5JtBkvCFKdEsdUm"));
        txOutputs.push_back(createTxOut(1747000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "000000000000000100000002540be400000000");
    }
    {
        int nBlock = 0;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(87000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(6000, "C9SkYGdcnTnjMKc9pvSVkeHX2ctB2BLbnc"));
        txOutputs.push_back(createTxOut(6000, "6uxd4fdZ8wXeCPXaxxDohSn1afeTYEaxVc"));
        txOutputs.push_back(createTxOut(7000, "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ"));
        txOutputs.push_back(createTxOut(7000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getFeePaid(), 55000);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "000000000000000100000002540be400000000");
    }
    {
        int nBlock = ConsensusParams().SCRIPTHASH_BLOCK;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(100000, "UgaWSroMxX2Ub64sxAnFEHFXxBMrrFmeWx"));
        txInputs.push_back(createTxOut(100000, "UgaWSroMxX2Ub64sxAnFEHFXxBMrrFmeWx"));
        txInputs.push_back(createTxOut(200000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));
        txInputs.push_back(createTxOut(100000, "UgaWSroMxX2Ub64sxAnFEHFXxBMrrFmeWx"));
        txInputs.push_back(createTxOut(200000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(6000, "C9SkYGdcnTnjMKc9pvSVkeHX2ctB2BLbnc"));
        txOutputs.push_back(createTxOut(6000, "C9Y3DTkwCe2Rt7XCR7yZwfoudoknuohosM"));
        txOutputs.push_back(createTxOut(6001, "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ"));
        txOutputs.push_back(createTxOut(665999, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getFeePaid(), 10000);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "C9Y3DTkwCe2Rt7XCR7yZwfoudoknuohosM");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "000000000000000100000002540be400000000");
    }
    {
        int nBlock = 0;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(70000, "BsmJKGw167AYme4SPW2pzb1G7VV5s3p4o2"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(9001, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(9001, "BsmJKGw167AYme4SPW2pzb1G7VV5s3p4o2"));
        txOutputs.push_back(createTxOut(9001, "BsmJKGw167AYnP2qxXW7emqRdBTqYC9xLK"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "BsmJKGw167AYme4SPW2pzb1G7VV5s3p4o2");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "BsmJKGw167AYnP2qxXW7emqRdBTqYC9xLK");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "00000000000000010000000777777700000000");
    }
    {
        int nBlock = ConsensusParams().SCRIPTHASH_BLOCK;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(1815000, "Ug7egduWEAjzURB6v24L2p1hFXYJKRtNVK"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(6001, "UQMVXNYr7rekJt9zvA33EemXhipJ4VUQR1"));
        txOutputs.push_back(createTxOut(6002, "UQVxFqU49ez9gQTQA3UGD6CVrLuMpuaX8P"));
        txOutputs.push_back(createTxOut(6003, "UeQF4rR7eeiiMmf6cHj4z1g9dHYDc317Si"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "Ug7egduWEAjzURB6v24L2p1hFXYJKRtNVK");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "UQVxFqU49ez9gQTQA3UGD6CVrLuMpuaX8P");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "000000000000000200000002540be400000000");
    }
}

BOOST_AUTO_TEST_CASE(invalid_class_a)
{
    // More than one data packet
    {
        int nBlock = 0;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(1815000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(6000, "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ"));
        txOutputs.push_back(createTxOut(6000, "C4cWj6wnh7GhSTKJJVh5JtBkvCFKdEsdUm"));
        txOutputs.push_back(createTxOut(6000, "C4cWj6wnh7GhSTKJJVh5JtBkvCFKdEsdUm"));
        txOutputs.push_back(createTxOut(1747000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) != 0);
    }
    // Not MSC or TMSC
    {
        int nBlock = 0;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(1815000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(6000, "C5wJzTrjQwYAsDk8yPtfng5DBr7LRo3udr"));
        txOutputs.push_back(createTxOut(6000, "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ"));
        txOutputs.push_back(createTxOut(1747000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) != 0);
    }
    // Seq collision
    {
        int nBlock = 0;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(1815000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "CEXodUs3feFVbq2zfvBimFdpS4evGZq15c"));
        txOutputs.push_back(createTxOut(6000, "C9SkYGdcnTnjMKc9pvSVkeHX2ctB2BLbnc"));
        txOutputs.push_back(createTxOut(6000, "C9Y3DTkwCe2Rt7XCR7yZwfoudoknuohosM"));
        txOutputs.push_back(createTxOut(6000, "C4kYHmwRhj5ZgJdC2RYWKyujKfovZudFXJ"));
        txOutputs.push_back(createTxOut(1747000, "C9ajxeK8qzjbzZQxkTFWKw8vycfChdi6xi"));

        CTransaction dummyTx = TxClassA(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) != 0);
    }
}


BOOST_AUTO_TEST_SUITE_END()
