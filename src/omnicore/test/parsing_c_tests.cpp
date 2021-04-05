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
#include <util/strencodings.h>

#include <stdint.h>
#include <limits>
#include <vector>

#include <boost/test/unit_test.hpp>

using namespace mastercore;

BOOST_FIXTURE_TEST_SUITE(omnicore_parsing_c_tests, BasicTestingSetup)

/** Creates a dummy transaction with the given inputs and outputs. */
static CTransaction TxClassC(const std::vector<CTxOut>& txInputs, const std::vector<CTxOut>& txOuts)
{
    CMutableTransaction mutableTx;

    // Inputs:
    for (const auto& txOut : txInputs)
    {
        // Create transaction for input:
        CMutableTransaction inputTx;
        inputTx.vout.push_back(txOut);
        CTransaction tx(inputTx);

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

BOOST_AUTO_TEST_CASE(reference_identification)
{
    {
        int nBlock = ConsensusParams().NULLDATA_BLOCK;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(5000000, "C3mPrmQeD2wyZUea2PgSyndwJei4yvABgj"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(OpReturn_SimpleSend());
        txOutputs.push_back(createTxOut(2700000, EncodeDestination(ExodusAddress())));

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK(metaTx.getReceiver().empty());
        BOOST_CHECK_EQUAL(metaTx.getFeePaid(), 2300000);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "C3mPrmQeD2wyZUea2PgSyndwJei4yvABgj");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "00000000000000070000000006dac2c0");
    }
    {
        int nBlock = ConsensusParams().NULLDATA_BLOCK + 1000;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(6000, "UNdN4QCMEohRX8Zwi2frqfSMYdtm9izYzS"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(OpReturn_SimpleSend());
        txOutputs.push_back(createTxOut(6000, "UX1DWJfZgomoh3P8qbzgrzxDU8zGZRSTw5"));

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getFeePaid(), 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "UNdN4QCMEohRX8Zwi2frqfSMYdtm9izYzS");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "UX1DWJfZgomoh3P8qbzgrzxDU8zGZRSTw5");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "00000000000000070000000006dac2c0");
    }
    {
        int nBlock = std::numeric_limits<int>::max();

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(80000, "CAzV2VgxTMbxMB1quRuiDCXZKo3Hqbp8U8"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(OpReturn_SimpleSend());
        txOutputs.push_back(createTxOut(6000, "CAzV2VgxTMbxMB1quRuiDCXZKo3Hqbp8U8"));

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getFeePaid(), 74000);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "CAzV2VgxTMbxMB1quRuiDCXZKo3Hqbp8U8");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "CAzV2VgxTMbxMB1quRuiDCXZKo3Hqbp8U8");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "00000000000000070000000006dac2c0");
    }
    {
        int nBlock = std::numeric_limits<int>::max();

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(80000, "C3mPrmQeD2wyZUea2PgSyndwJei4yvABgj"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(OpReturn_SimpleSend());
        txOutputs.push_back(createTxOut(6000, "UTHebyswtaWFcidNfyPvrLGSLehCyWU3bm"));
        txOutputs.push_back(PayToPubKey_Unrelated());
        txOutputs.push_back(NonStandardOutput());
        txOutputs.push_back(createTxOut(6000, "UQhW2UBJMS17E1JK9vum5oUwDgZE3rYfpv"));
        txOutputs.push_back(PayToBareMultisig_1of3());
        txOutputs.push_back(createTxOut(6000, "C3mPrmQeD2wyZUea2PgSyndwJei4yvABgj"));

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "C3mPrmQeD2wyZUea2PgSyndwJei4yvABgj");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "UQhW2UBJMS17E1JK9vum5oUwDgZE3rYfpv");
    }
    {
        int nBlock = std::numeric_limits<int>::max();

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(55550, "UVpwGR2hhHgbwpcTm7a1gZAAaZCtKqLc4N"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(createTxOut(6000, "UTHebyswtaWFcidNfyPvrLGSLehCyWU3bm"));
        txOutputs.push_back(PayToPubKey_Unrelated());
        txOutputs.push_back(NonStandardOutput());
        txOutputs.push_back(createTxOut(6000, "UVpwGR2hhHgbwpcTm7a1gZAAaZCtKqLc4N"));
        txOutputs.push_back(createTxOut(6000, "UVpwGR2hhHgbwpcTm7a1gZAAaZCtKqLc4N"));
        txOutputs.push_back(PayToPubKeyHash_Exodus());
        txOutputs.push_back(OpReturn_SimpleSend());

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "UVpwGR2hhHgbwpcTm7a1gZAAaZCtKqLc4N");
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "UVpwGR2hhHgbwpcTm7a1gZAAaZCtKqLc4N");
    }
}

BOOST_AUTO_TEST_CASE(empty_op_return)
{
    {
        int nBlock = std::numeric_limits<int>::max();

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(900000, "UVpwGR2hhHgbwpcTm7a1gZAAaZCtKqLc4N"));

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(OpReturn_PlainMarker());
        txOutputs.push_back(PayToPubKeyHash_Unrelated());

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK(metaTx.getPayload().empty());
        BOOST_CHECK_EQUAL(metaTx.getSender(), "UVpwGR2hhHgbwpcTm7a1gZAAaZCtKqLc4N");
        // via PayToPubKeyHash_Unrelated:
        BOOST_CHECK_EQUAL(metaTx.getReceiver(), "C2uS5SDveHLU4oecepg8XJuizD3pMDs2m5");
    }
}


BOOST_AUTO_TEST_CASE(trimmed_op_return)
{
    {
        int nBlock = std::numeric_limits<int>::max();

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(100000, "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y"));

        std::vector<CTxOut> txOutputs;

        std::vector<unsigned char> vchFiller(MAX_PACKETS * PACKET_SIZE, 0x07);
        std::vector<unsigned char> vchPayload = GetOmMarker();
        vchPayload.insert(vchPayload.end(), vchFiller.begin(), vchFiller.end());

        // These will be trimmed:
        vchPayload.push_back(0x44);
        vchPayload.push_back(0x44);
        vchPayload.push_back(0x44);

        CScript scriptPubKey;
        scriptPubKey << OP_RETURN;
        scriptPubKey << vchPayload;
        CTxOut txOut = CTxOut(0, scriptPubKey);
        txOutputs.push_back(txOut);

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), HexStr(vchFiller.begin(), vchFiller.end()));
        BOOST_CHECK_EQUAL(metaTx.getPayload().size() / 2, MAX_PACKETS * PACKET_SIZE);
    }
}

BOOST_AUTO_TEST_CASE(multiple_op_return_short)
{
    {
        int nBlock = ConsensusParams().NULLDATA_BLOCK;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(100000, "C3mPrmQeD2wyZUea2PgSyndwJei4yvABgj"));

        std::vector<CTxOut> txOutputs;
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN << ParseHex("6f6d6e690000111122223333");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN;
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN << ParseHex("6f6d6e690001000200030004");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN << ParseHex("6f6d6e69");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }
        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "C3mPrmQeD2wyZUea2PgSyndwJei4yvABgj");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "00001111222233330001000200030004");
    }
}

BOOST_AUTO_TEST_CASE(multiple_op_return)
{
    {
        int nBlock = ConsensusParams().NULLDATA_BLOCK;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(100000, "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y"));

        std::vector<CTxOut> txOutputs;
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN << ParseHex("6f6d6e691222222222222222222222222223");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN << ParseHex("6f6d6e694555555555555555555555555556");
            CTxOut txOut = CTxOut(5, scriptPubKey);
            txOutputs.push_back(txOut);
        }
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN << ParseHex("6f6d6e69788888888889");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }
        {
            CScript scriptPubKey; // has no marker and will be ignored!
            scriptPubKey << OP_RETURN << ParseHex("4d756c686f6c6c616e64204472697665");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN << ParseHex("6f6d6e69ffff11111111111111111111"
                    "11111111111111111111111111111111111111111111111111111111111111"
                    "11111111111111111111111111111111111111111111111111111111111111"
                    "111111111111111111111111111111111111111111111117");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y");
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "12222222222222222222222222234555555"
                "555555555555555555556788888888889ffff11111111111111111111111111111"
                "111111111111111111111111111111111111111111111111111111111111111111"
                "111111111111111111111111111111111111111111111111111111111111111111"
                "1111111111111111111111111111117");
    }
}

BOOST_AUTO_TEST_CASE(multiple_op_return_pushes)
{
    {
        int nBlock = std::numeric_limits<int>::max();

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(100000, "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y"));
        txInputs.push_back(PayToBareMultisig_3of5());

        std::vector<CTxOut> txOutputs;
        txOutputs.push_back(OpReturn_SimpleSend());
        txOutputs.push_back(PayToScriptHash_Unrelated());
        txOutputs.push_back(OpReturn_MultiSimpleSend());

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y");
        BOOST_CHECK_EQUAL(metaTx.getPayload(),
                // OpReturn_SimpleSend (without marker):
                "00000000000000070000000006dac2c0"
                // OpReturn_MultiSimpleSend (without marker):
                "00000000000000070000000000002329"
                "0062e907b15cbf27d5425399ebf6f0fb50ebb88f18"
                "000000000000001f0000000001406f40"
                "05da59767e81f4b019fe9f5984dbaa4f61bf197967");
    }
    {
        int nBlock = ConsensusParams().NULLDATA_BLOCK;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(100000, "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y"));

        std::vector<CTxOut> txOutputs;
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN;
            scriptPubKey << ParseHex("6f6d6e6900000000000000010000000006dac2c0");
            scriptPubKey << ParseHex("00000000000000030000000000000d48");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getSender(), "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y");
        BOOST_CHECK_EQUAL(metaTx.getPayload(),
                "00000000000000010000000006dac2c000000000000000030000000000000d48");
    }
    {
        int nBlock = std::numeric_limits<int>::max();

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(100000, "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y"));

        std::vector<CTxOut> txOutputs;
        {
          CScript scriptPubKey;
          scriptPubKey << OP_RETURN;
          scriptPubKey << ParseHex("6f6d6e69");
          scriptPubKey << ParseHex("00000000000000010000000006dac2c0");
          CTxOut txOut = CTxOut(0, scriptPubKey);
          txOutputs.push_back(txOut);
        }

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0);
        BOOST_CHECK_EQUAL(metaTx.getPayload(), "00000000000000010000000006dac2c0");
    }
    {
        /**
         * The following transaction is invalid, because the first pushed data
         * doesn't contain the class C marker.
         */
        int nBlock = ConsensusParams().NULLDATA_BLOCK;

        std::vector<CTxOut> txInputs;
        txInputs.push_back(createTxOut(100000, "UeZaknatSAkc3BW1bKgW78BBY4S9eqzw2Y"));

        std::vector<CTxOut> txOutputs;
        {
            CScript scriptPubKey;
            scriptPubKey << OP_RETURN;
            scriptPubKey << ParseHex("6f6d");
            scriptPubKey << ParseHex("6e69");
            scriptPubKey << ParseHex("00000000000000010000000006dac2c0");
            CTxOut txOut = CTxOut(0, scriptPubKey);
            txOutputs.push_back(txOut);
        }

        CTransaction dummyTx = TxClassC(txInputs, txOutputs);

        CMPTransaction metaTx;
        BOOST_CHECK(ParseTransaction(dummyTx, nBlock, 1, metaTx) != 0);
    }
}


BOOST_AUTO_TEST_SUITE_END()
