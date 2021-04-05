#include <omnicore/test/utils_tx.h>

#include <omnicore/omnicore.h>
#include <omnicore/parsing.h>
#include <omnicore/script.h>
#include <omnicore/tx.h>

#include <base58.h>
#include <coins.h>
#include <key_io.h>
#include <primitives/transaction.h>
#include <random.h>
#include <script/script.h>
#include <script/standard.h>
#include <test/test_bitcoin.h>

#include <stdint.h>
#include <algorithm>
#include <limits>
#include <vector>

#include <boost/test/unit_test.hpp>

using namespace mastercore;

BOOST_FIXTURE_TEST_SUITE(omnicore_sender_bycontribution_tests, BasicTestingSetup)

// Forward declarations
static CTransaction TxClassB(const std::vector<CTxOut>& txInputs);
static bool GetSenderByContribution(const std::vector<CTxOut>& vouts, std::string& strSender);
static CTxOut createTxOut(int64_t amount, const std::string& dest);
static CKeyID createRandomKeyId();
static CScriptID createRandomScriptId();
void shuffleAndCheck(std::vector<CTxOut>& vouts, unsigned nRounds);

// Test settings
static const unsigned nOutputs = 256;
static const unsigned nAllRounds = 2;
static const unsigned nShuffleRounds = 16;

/**
 * Tests the invalidation of the transaction, when there are not allowed inputs.
 */
BOOST_AUTO_TEST_CASE(invalid_inputs)
{
    {
        std::vector<CTxOut> vouts;
        vouts.push_back(PayToPubKey_Unrelated());
        vouts.push_back(PayToPubKeyHash_Unrelated());
        std::string strSender;
        BOOST_CHECK(!GetSenderByContribution(vouts, strSender));
    }
    {
        std::vector<CTxOut> vouts;
        vouts.push_back(PayToPubKeyHash_Unrelated());
        vouts.push_back(PayToBareMultisig_1of3());
        std::string strSender;
        BOOST_CHECK(!GetSenderByContribution(vouts, strSender));
    }
    {
        std::vector<CTxOut> vouts;
        vouts.push_back(PayToScriptHash_Unrelated());
        vouts.push_back(PayToPubKeyHash_Exodus());
        vouts.push_back(NonStandardOutput());
        std::string strSender;
        BOOST_CHECK(!GetSenderByContribution(vouts, strSender));
    }
}

/**
 * Tests sender selection "by sum" with pay-to-pubkey-hash outputs, where a single
 * candidate has the highest output value.
 */
BOOST_AUTO_TEST_CASE(p2pkh_contribution_by_sum_test)
{
    std::vector<CTxOut> vouts;
    vouts.push_back(createTxOut(100, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(100, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(100, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(100, "C9f2wm9DXgtfgwXrxhS5xhAjU75tgT557E"));
    vouts.push_back(createTxOut(100, "C9f2wm9DXgtfgwXrxhS5xhAjU75tgT557E"));
    vouts.push_back(createTxOut(999, "CG3inEzV9BUmPkeoWNuDTDUJzczMTAhucn")); // Winner
    vouts.push_back(createTxOut(100, "C8xpwP6s4P6mXqPeh1Djw4HzgRcVE11pUh"));
    vouts.push_back(createTxOut(100, "C8xpwP6s4P6mXqPeh1Djw4HzgRcVE11pUh"));
    vouts.push_back(createTxOut(100, "C44cbjkC66xTi4PzeFHYoNBNxStxPaf2XK"));

    std::string strExpected("CG3inEzV9BUmPkeoWNuDTDUJzczMTAhucn");

    for (int i = 0; i < 10; ++i) {
        std::random_shuffle(vouts.begin(), vouts.end(), GetRandInt);

        std::string strSender;
        BOOST_CHECK(GetSenderByContribution(vouts, strSender));
        BOOST_CHECK_EQUAL(strExpected, strSender);
    }
}

/**
 * Tests sender selection "by sum" with pay-to-pubkey-hash outputs, where a candidate
 * with the highest output value by sum, with more than one output, is chosen.
 */
BOOST_AUTO_TEST_CASE(p2pkh_contribution_by_total_sum_test)
{
    std::vector<CTxOut> vouts;
    vouts.push_back(createTxOut(499, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(501, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(295, "C44cbjkC66xTi4PzeFHYoNBNxStxPaf2XK")); // Winner
    vouts.push_back(createTxOut(310, "C44cbjkC66xTi4PzeFHYoNBNxStxPaf2XK")); // Winner
    vouts.push_back(createTxOut(400, "C44cbjkC66xTi4PzeFHYoNBNxStxPaf2XK")); // Winner
    vouts.push_back(createTxOut(500, "BwFYgknrvkQf47srLYBL9YdpXHAPtkqYHQ"));
    vouts.push_back(createTxOut(500, "BwFYgknrvkQf47srLYBL9YdpXHAPtkqYHQ"));

    std::string strExpected("C44cbjkC66xTi4PzeFHYoNBNxStxPaf2XK");

    for (int i = 0; i < 10; ++i) {
        std::random_shuffle(vouts.begin(), vouts.end(), GetRandInt);

        std::string strSender;
        BOOST_CHECK(GetSenderByContribution(vouts, strSender));
        BOOST_CHECK_EQUAL(strExpected, strSender);
    }
}

/**
 * Tests sender selection "by sum" with pay-to-pubkey-hash outputs, where all outputs
 * have equal values, and a candidate is chosen based on the lexicographical order of
 * the base58 string representation (!) of the candidate.
 *
 * Note: it reflects the behavior of Omni Core, but this edge case is not specified.
 */
BOOST_AUTO_TEST_CASE(p2pkh_contribution_by_sum_order_test)
{
    std::vector<CTxOut> vouts;
    vouts.push_back(createTxOut(1000, "BwFYgknrvkQf47srLYBL9YdpXHAPtkqYHQ")); // Winner
    vouts.push_back(createTxOut(1000, "C9f2wm9DXgtfgwXrxhS5xhAjU75tgT557E"));
    vouts.push_back(createTxOut(1000, "CG3inEzV9BUmPkeoWNuDTDUJzczMTAhucn"));
    vouts.push_back(createTxOut(1000, "C8xpwP6s4P6mXqPeh1Djw4HzgRcVE11pUh"));
    vouts.push_back(createTxOut(1000, "C44cbjkC66xTi4PzeFHYoNBNxStxPaf2XK"));
    vouts.push_back(createTxOut(1000, "C9qEU5sFUnMq3LJ6osaiUXYt26nck9TS24"));
    vouts.push_back(createTxOut(1000, "ByfpMBAJBxutpwBjYkidKCabP65phsZZ8a"));
    vouts.push_back(createTxOut(1000, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(1000, "C8N8DJhzD15qPLZo8Q23KgFVP57F8p1YPb"));

    std::string strExpected("BwFYgknrvkQf47srLYBL9YdpXHAPtkqYHQ");

    for (int i = 0; i < 10; ++i) {
        std::random_shuffle(vouts.begin(), vouts.end(), GetRandInt);

        std::string strSender;
        BOOST_CHECK(GetSenderByContribution(vouts, strSender));
        BOOST_CHECK_EQUAL(strExpected, strSender);
    }
}

/**
 * Tests sender selection "by sum" with pay-to-script-hash outputs, where a single
 * candidate has the highest output value.
 */
BOOST_AUTO_TEST_CASE(p2sh_contribution_by_sum_test)
{
    std::vector<CTxOut> vouts;
    vouts.push_back(createTxOut(100, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(150, "UU7Uag2CfFZwnWZnu1s75XEZDTajKZSSiP"));
    vouts.push_back(createTxOut(400, "Uf5iMMGLgsQTjWk3vcYwDeZFpGxfj5VLA2"));
    vouts.push_back(createTxOut(100, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(400, "UPMdqVyQ6xjkCXXX4zW2NL2mKPuMiknmRk"));
    vouts.push_back(createTxOut(100, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(777, "UkyQxRd4Ft5vEaJcbGWGGW4HX5u6VXi8LJ")); // Winner
    vouts.push_back(createTxOut(100, "Ud1A1dHBEQUmBWqE6ajwMMwZx1kvGuhV76"));

    std::string strExpected("UkyQxRd4Ft5vEaJcbGWGGW4HX5u6VXi8LJ");

    for (int i = 0; i < 10; ++i) {
        std::random_shuffle(vouts.begin(), vouts.end(), GetRandInt);

        std::string strSender;
        BOOST_CHECK(GetSenderByContribution(vouts, strSender));
        BOOST_CHECK_EQUAL(strExpected, strSender);
    }
}

/**
 * Tests sender selection "by sum" with pay-to-pubkey-hash and pay-to-script-hash 
 * outputs mixed, where a candidate with the highest output value by sum, with more 
 * than one output, is chosen.
 */
BOOST_AUTO_TEST_CASE(p2sh_contribution_by_total_sum_test)
{
    std::vector<CTxOut> vouts;

    vouts.push_back(createTxOut(100, "Uf5iMMGLgsQTjWk3vcYwDeZFpGxfj5VLA2"));
    vouts.push_back(createTxOut(500, "Uf5iMMGLgsQTjWk3vcYwDeZFpGxfj5VLA2"));
    vouts.push_back(createTxOut(600, "UXto74uxrqBZ3WVkQiT5EMYpvbioJEr7Nv")); // Winner
    vouts.push_back(createTxOut(500, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(100, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));
    vouts.push_back(createTxOut(350, "UXto74uxrqBZ3WVkQiT5EMYpvbioJEr7Nv")); // Winner
    vouts.push_back(createTxOut(110, "C2myZcxhdVfq6n364EgatYEdgdmxDTjrHj"));

    std::string strExpected("UXto74uxrqBZ3WVkQiT5EMYpvbioJEr7Nv");

    for (int i = 0; i < 10; ++i) {
        std::random_shuffle(vouts.begin(), vouts.end(), GetRandInt);

        std::string strSender;
        BOOST_CHECK(GetSenderByContribution(vouts, strSender));
        BOOST_CHECK_EQUAL(strExpected, strSender);
    }
}

/**
 * Tests sender selection "by sum" with pay-to-script-hash outputs, where all outputs
 * have equal values, and a candidate is chosen based on the lexicographical order of
 * the base58 string representation (!) of the candidate.
 *
 * Note: it reflects the behavior of Omni Core, but this edge case is not specified.
 */
BOOST_AUTO_TEST_CASE(p2sh_contribution_by_sum_order_test)
{
    std::vector<CTxOut> vouts;
    vouts.push_back(createTxOut(1000, "UPMdqVyQ6xjkCXXX4zW2NL2mKPuMiknmRk")); // Winner
    vouts.push_back(createTxOut(1000, "Uf5iMMGLgsQTjWk3vcYwDeZFpGxfj5VLA2"));
    vouts.push_back(createTxOut(1000, "UXto74uxrqBZ3WVkQiT5EMYpvbioJEr7Nv"));
    vouts.push_back(createTxOut(1000, "UPs6kD2zXQfo31fgTDAw6xsd6TwVg1CBT9"));
    vouts.push_back(createTxOut(1000, "UU7Uag2CfFZwnWZnu1s75XEZDTajKZSSiP"));
    vouts.push_back(createTxOut(1000, "UkyQxRd4Ft5vEaJcbGWGGW4HX5u6VXi8LJ"));
    vouts.push_back(createTxOut(1000, "Ud1A1dHBEQUmBWqE6ajwMMwZx1kvGuhV76"));
    vouts.push_back(createTxOut(1000, "UfzhoVzoy44SXrpifD44NGZVXeLWHR9n8p"));
    vouts.push_back(createTxOut(1000, "Ug4wp7kTSXqAwMYDu4bdBT1wQE3R7yvzcc"));
    
    std::string strExpected("UPMdqVyQ6xjkCXXX4zW2NL2mKPuMiknmRk");

    for (int i = 0; i < 10; ++i) {
        std::random_shuffle(vouts.begin(), vouts.end(), GetRandInt);

        std::string strSender;
        BOOST_CHECK(GetSenderByContribution(vouts, strSender));
        BOOST_CHECK_EQUAL(strExpected, strSender);
    }
}

/**
 * Tests sender selection "by sum", where the lexicographical order of the base58 
 * representation as string (instead of uint160) determines the chosen candidate.
 *
 * In practise this implies selecting the sender "by sum" via a comparison of 
 * CTxDestination objects would yield faulty results.
 *
 * Note: it reflects the behavior of Omni Core, but this edge case is not specified.
 */
/*
BOOST_AUTO_TEST_CASE(sender_selection_string_based_test)
{
    std::vector<CTxOut> vouts;
    // Hash 160: 539a7a290034e0107f27cb36cb2d1095b1768d10
    vouts.push_back(createTxOut(1000, "UVc56a1LsSiNvZZ3LSQZJFgY6DD6gXozLC")); // Winner
    // Hash 160: 70161ebf72f894fbee554e88cf7889035899827f
    vouts.push_back(createTxOut(1000, "UYCg8oTkKmhUu1mWEhHe3TKEV52hiojNJB"));
    // Hash 160: b94b91c9e0423f6e5279de68989dda9032d67e87
    vouts.push_back(createTxOut(1000, "UesmVXtWiSjzXADu5aB84UEoNDW4qf9uss"));
    // Hash 160: 06569c8f59f428db748c94bf1d5d9f5f9d0db116
    vouts.push_back(createTxOut(1000, "UNZXmQtEVCeeqxfpqSmxv3ZmbyUJ3ejoCZ"));  // Not!

    std::string strExpected("UVc56a1LsSiNvZZ3LSQZJFgY6DD6gXozLC");

    for (int i = 0; i < 24; ++i) {
        std::random_shuffle(vouts.begin(), vouts.end(), GetRandInt);

        std::string strSender;
        BOOST_CHECK(GetSenderByContribution(vouts, strSender));
        BOOST_CHECK_EQUAL(strExpected, strSender);
    }
}
*/

/**
 * Tests order independence of the sender selection "by sum" for pay-to-pubkey-hash
 * outputs, where all output values are equal.
 */
BOOST_AUTO_TEST_CASE(sender_selection_same_amount_test)
{
    for (unsigned i = 0; i < nAllRounds; ++i) {
        std::vector<CTxOut> vouts;
        for (unsigned n = 0; n < nOutputs; ++n) {
            CTxOut output(static_cast<int64_t>(1000),
                    GetScriptForDestination(createRandomKeyId()));
            vouts.push_back(output);
        }
        shuffleAndCheck(vouts, nShuffleRounds);
    }
}

/**
 * Tests order independence of the sender selection "by sum" for pay-to-pubkey-hash
 * outputs, where output values are different for each output.
 */
BOOST_AUTO_TEST_CASE(sender_selection_increasing_amount_test)
{
    for (unsigned i = 0; i < nAllRounds; ++i) {
        std::vector<CTxOut> vouts;
        for (unsigned n = 0; n < nOutputs; ++n) {
            CTxOut output(static_cast<int64_t>(1000 + n),
                    GetScriptForDestination(createRandomKeyId()));
            vouts.push_back(output);
        }
        shuffleAndCheck(vouts, nShuffleRounds);
    }
}

/**
 * Tests order independence of the sender selection "by sum" for pay-to-pubkey-hash
 * and pay-to-script-hash outputs mixed together, where output values are equal for 
 * every second output.
 */
BOOST_AUTO_TEST_CASE(sender_selection_mixed_test)
{
    for (unsigned i = 0; i < nAllRounds; ++i) {
        std::vector<CTxOut> vouts;
        for (unsigned n = 0; n < nOutputs; ++n) {
            CScript scriptPubKey;
            if (GetRandInt(2) == 0) {
                scriptPubKey = GetScriptForDestination(createRandomKeyId());
            } else {
                scriptPubKey = GetScriptForDestination(createRandomScriptId());
            };
            int64_t nAmount = static_cast<int64_t>(1000 - n * (n % 2 == 0));
            vouts.push_back(CTxOut(nAmount, scriptPubKey));
        }
        shuffleAndCheck(vouts, nShuffleRounds);
    }
}

/** Creates a dummy class B transaction with the given inputs. */
static CTransaction TxClassB(const std::vector<CTxOut>& txInputs)
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

    // Outputs:
    mutableTx.vout.push_back(PayToPubKeyHash_Exodus());
    mutableTx.vout.push_back(PayToBareMultisig_1of3());
    mutableTx.vout.push_back(PayToPubKeyHash_Unrelated());

    return CTransaction(mutableTx);
}

/** Extracts the sender "by contribution". */
static bool GetSenderByContribution(const std::vector<CTxOut>& vouts, std::string& strSender)
{
    int nBlock = std::numeric_limits<int>::max();

    CMPTransaction metaTx;
    CTransaction dummyTx = TxClassB(vouts);

    if (ParseTransaction(dummyTx, nBlock, 1, metaTx) == 0) {
        strSender = metaTx.getSender();
        return true;
    }

    return false;
}

/** Helper to create a CTxOut object. */
static CTxOut createTxOut(int64_t amount, const std::string& dest)
{
    return CTxOut(amount, GetScriptForDestination(DecodeDestination(dest)));
}

/** Helper to create a CKeyID object with random value.*/
static CKeyID createRandomKeyId()
{
    std::vector<unsigned char> vch;
    vch.reserve(20);
    for (int i = 0; i < 20; ++i) {
        vch.push_back(static_cast<unsigned char>(GetRandInt(256)));
    }
    return CKeyID(uint160(vch));
}

/** Helper to create a CScriptID object with random value.*/
static CScriptID createRandomScriptId()
{
    std::vector<unsigned char> vch;
    vch.reserve(20);
    for (int i = 0; i < 20; ++i) {
        vch.push_back(static_cast<unsigned char>(GetRandInt(256)));
    }
    return CScriptID(uint160(vch));
}

/**
 * Identifies the sender of a transaction, based on the list of provided transaction
 * outputs, and then shuffles the list n times, while checking, if this produces the
 * same result. The "contribution by sum" sender selection doesn't require specific
 * positions or order of outputs, and should work in all cases.
 */
void shuffleAndCheck(std::vector<CTxOut>& vouts, unsigned nRounds)
{
    std::string strSenderFirst;
    BOOST_CHECK(GetSenderByContribution(vouts, strSenderFirst));

    for (unsigned j = 0; j < nRounds; ++j) {
        std::random_shuffle(vouts.begin(), vouts.end(), GetRandInt);

        std::string strSender;
        BOOST_CHECK(GetSenderByContribution(vouts, strSender));
        BOOST_CHECK_EQUAL(strSenderFirst, strSender);
    }
}


BOOST_AUTO_TEST_SUITE_END()
