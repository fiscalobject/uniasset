// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <tinyformat.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <versionbitsinfo.h>

#include <assert.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "2 january 2014";
    const CScript genesisOutputScript = CScript() << 0x0 << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 400000;
        consensus.BIP34Height = 146557;
        consensus.BIP34v2Height = 160054;
        consensus.BIP34Hash = uint256S("0xee9812510f877abf44332294a0b72673cabcd8f479fa678abd126099533a6c96");
        consensus.BIP65Height = 1205150;
        consensus.BIP66Height = 1205150;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.checkpointPubKey = "044318157bd82b6e17926c8804eecf73140f416c34ccc2237c56614d081dce88a98293b40891d801d16a2899defe7869706d7ec55118ef8f06c683cfdc6b6a8c58";
        consensus.nPowTargetTimespan = 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 90;
        consensus.nCoinFix = 15000;
        consensus.nHardForkOne = 33479;
        consensus.nHardForkTwo = 160997;
        consensus.nHardForkTwoA = 171900;
        consensus.nHardForkThree = 266000;
        consensus.nHardForkFour = 1182000;
        consensus.nHardForkFourA = 1220000;
        consensus.nNeoScryptFork = 1414446393;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 10080; // 75% of 13440
        consensus.nMinerConfirmationWindow = 13440;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1517443200; // Feb 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1548979200; // Feb 1st, 2019

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1519862400; // Mar 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1551398400; // Mar 1st, 2019

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000004a1a916f120b7d");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x3546eca460b23437d2650501d231570293552f3145a82f053ecb4c85dd23e65b"); //563378

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xd9;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdd;
        nDefaultPort = 9887;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 2;
        m_assumed_chain_state_size = 1;

        genesis = CreateGenesisBlock(1388681920, 1671824, 0x1e0ffff0, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0xba1d39b4928ab03d813d952daf65fb7797fcf538a9c1b8274f4edc8557722d13"));
        assert(genesis.hashMerkleRoot == uint256S("0x8207df3a28a5bfdcaba0c810e540123aaea8d067b745092849787169f5e77065"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        vSeeds.emplace_back("dns.seed1.ufocoin.net");
        vSeeds.emplace_back("dns.seed2.ufocoin.net");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,27);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,68);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,155);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "uf";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                {    4500, uint256S("0x5755857a8055c732d5236b0526afcb9b92f1291c87ed3c655c6d79df6b9d3dd4")},
                {    9999, uint256S("0x808bf9bdf3c7e777ad8008455f6849001bc264910de86e01a0bf1d83ed362aba")},
                {   20000, uint256S("0xe14a9e1d1cd79fa0385d3af7eac36ed96f29d7c0205b62eb82c4e7c5b043c6d1")},
                {   33349, uint256S("0xcf9ea4ab6589b0ac0cc34fca94ea3c24842ac80f43724d0c8d89ece0aa0a5081")},
                { 1079136, uint256S("0xe171e30fa1ab3428f079a165a22f5cfd3529fb0e76bd0e7213a3ac9a09bd5571")},
                { 1213947, uint256S("0xa4c7b570fbf1d755c327ff9c3d98e9d5433e453f9ecade20a8e4852bd124eb8f")},
                { 1246467, uint256S("0xa0e2460c7e644cbb6c4bc01088094524fdf90892aa42d22f9dd9b6e3c981ca6f")},
                { 1314322, uint256S("0xe4d06a126e75abe493b1d07e3c2112a9121ba9e9d66ec82da53acd48196b05fc")},
                { 1687176, uint256S("0x3546eca460b23437d2650501d231570293552f3145a82f053ecb4c85dd23e65b")},
            }
        };

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats 4096 0000000000000000000f1c54590ee18d15ec70e68c8cd4cfbadb1b4f11697eee
            /* nTime    */ 1564378049,
            /* nTxCount */ 2058605,
            /* dTxRate  */ 0.001
        };

        /* disable fallback fee on mainnet */
        m_fallback_fee_enabled = true;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 400000;
        consensus.BIP34Height = 0;
        consensus.BIP34v2Height = 0;
        consensus.BIP34Hash = uint256S("0x45b4e55bddf20dfeb69ef2a35dd36f58dd45d5f4582c1a4ca1c1b78eef8f8c37");
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.powLimit = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.checkpointPubKey = "04d0dd87fbb6ac3483f57c9cd010c8fa804219ec641fad97a9cbb31605327b15fa9c40232fa214f02b80883955f7b14e49dbd03e44d45123f06ee08b911a08be33";
        consensus.nPowTargetTimespan = 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 90;
        consensus.nHardForkTwo = 1;
        consensus.nHardForkTwoA = 1;
        consensus.nHardForkThree = 1;
        consensus.nHardForkFour = 1100;
        consensus.nHardForkFourA = 1500;
        consensus.nNeoScryptFork = 1506816000;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 375; // 75% for 500
        consensus.nMinerConfirmationWindow = 500;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // // Deployment of BIP68, BIP112, and BIP113.
        // consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        // consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1516924800; // Jan 26th, 2018
        // consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1548460800; // Jan 26th, 2019

        // // Deployment of SegWit (BIP141, BIP143, and BIP147)
        // consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        // consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1519862400; // Mar 1st, 2018
        // consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1551398400; // Mar 1st, 2019

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::ALWAYS_ACTIVE;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::ALWAYS_ACTIVE;


        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000100010");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x45b4e55bddf20dfeb69ef2a35dd36f58dd45d5f4582c1a4ca1c1b78eef8f8c37"); //1354312

        pchMessageStart[0] = 0xfb;
        pchMessageStart[1] = 0xc0;
        pchMessageStart[2] = 0xb8;
        pchMessageStart[3] = 0xdb;
        nDefaultPort = 19887;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 2;

        genesis = CreateGenesisBlock(1388678813, 616291, 0x1e0ffff0, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x45b4e55bddf20dfeb69ef2a35dd36f58dd45d5f4582c1a4ca1c1b78eef8f8c37"));
        assert(genesis.hashMerkleRoot == uint256S("0x8207df3a28a5bfdcaba0c810e540123aaea8d067b745092849787169f5e77065"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.emplace_back("testnet-seed.ufocoin.net");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "ut";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        checkpointData = {
            {
                {0, uint256S("45b4e55bddf20dfeb69ef2a35dd36f58dd45d5f4582c1a4ca1c1b78eef8f8c37")},
            }
        };

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats 4096 0000000000000037a8cd3e06cd5edbfe9dd1dbcc5dacab279376ef7cfc2b4c75
            /* nTime    */ 1388678813,
            /* nTxCount */ 1,
            /* dTxRate  */ 0.001
        };

        /* enable fallback fee on testnet */
        m_fallback_fee_enabled = true;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    explicit CRegTestParams(const ArgsManager& args) {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 500; // BIP34 activated on regtest (Used in functional tests)
        consensus.BIP34v2Height = 500;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in functional tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in functional tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.checkpointPubKey = "04d0dd87fbb6ac3483f57c9cd010c8fa804219ec641fad97a9cbb31605327b15fa9c40232fa214f02b80883955f7b14e49dbd03e44d45123f06ee08b911a08be33";
        consensus.nPowTargetTimespan = 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 90;
        consensus.nHardForkTwoA = 1;
        consensus.nHardForkTwo = 1;
        consensus.nHardForkThree = 1;
        consensus.nHardForkFour = 1;
        consensus.nHardForkFourA = 1;
        consensus.nNeoScryptFork = 1524473955;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0x1b;
        pchMessageStart[1] = 0x21;
        pchMessageStart[2] = 0x55;
        pchMessageStart[3] = 0x1c;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        UpdateVersionBitsParametersFromArgs(args);

        genesis = CreateGenesisBlock(1296688602, 3, 0x207fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0xa482cf37ea99d8c74f62e28903208bfbc12901b35738feff20fdf7e3b671afb7"));
        assert(genesis.hashMerkleRoot == uint256S("0x8207df3a28a5bfdcaba0c810e540123aaea8d067b745092849787169f5e77065"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("a482cf37ea99d8c74f62e28903208bfbc12901b35738feff20fdf7e3b671afb7")},
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "ufrt";

        /* enable fallback fee on regtest */
        m_fallback_fee_enabled = true;
    }

    /**
     * Allows modifying the Version Bits regtest parameters.
     */
    void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
    void UpdateVersionBitsParametersFromArgs(const ArgsManager& args);
};

void CRegTestParams::UpdateVersionBitsParametersFromArgs(const ArgsManager& args)
{
    if (!args.IsArgSet("-vbparams")) return;

    for (const std::string& strDeployment : args.GetArgs("-vbparams")) {
        std::vector<std::string> vDeploymentParams;
        boost::split(vDeploymentParams, strDeployment, boost::is_any_of(":"));
        if (vDeploymentParams.size() != 3) {
            throw std::runtime_error("Version bits parameters malformed, expecting deployment:start:end");
        }
        int64_t nStartTime, nTimeout;
        if (!ParseInt64(vDeploymentParams[1], &nStartTime)) {
            throw std::runtime_error(strprintf("Invalid nStartTime (%s)", vDeploymentParams[1]));
        }
        if (!ParseInt64(vDeploymentParams[2], &nTimeout)) {
            throw std::runtime_error(strprintf("Invalid nTimeout (%s)", vDeploymentParams[2]));
        }
        bool found = false;
        for (int j=0; j < (int)Consensus::MAX_VERSION_BITS_DEPLOYMENTS; ++j) {
            if (vDeploymentParams[0] == VersionBitsDeploymentInfo[j].name) {
                UpdateVersionBitsParameters(Consensus::DeploymentPos(j), nStartTime, nTimeout);
                found = true;
                LogPrintf("Setting version bits activation parameters for %s to start=%ld, timeout=%ld\n", vDeploymentParams[0], nStartTime, nTimeout);
                break;
            }
        }
        if (!found) {
            throw std::runtime_error(strprintf("Invalid deployment (%s)", vDeploymentParams[0]));
        }
    }
}

static std::unique_ptr<const CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<const CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams(gArgs));
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}
