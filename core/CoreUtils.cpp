#include "CoreUtils.hpp"

namespace olympia::coreutils
{
    sparta::ParameterSet* getCoreExtensions(sparta::TreeNode* node)
    {
        auto orig_node = node;

        // Look for the node with the extension.  This will the core
        // in full simulation or the given node for testers
        while (node && (nullptr == node->getExtension(olympia::CoreExtensions::name)))
        {
            node = node->getParent();
        }

        if (nullptr == node)
        {
            node = orig_node;
        }

        auto core_extension = sparta::notNull(node)->createExtension(olympia::CoreExtensions::name);
        auto core_extension_params = sparta::notNull(core_extension)->getParameters();
        return sparta::notNull(core_extension_params);
    }

    uint32_t getVlenDlenRatio(sparta::TreeNode* node)
    {
        const auto vlen = getCoreExtensions(node)->getParameterValueAs<uint32_t>("vlen");
        if (vlen != 0)
        {
            const auto dlen = getCoreExtensions(node)->getParameterValueAs<uint32_t>("dlen");
            return vlen / dlen;
        }
        else
        {
            return 1;
        }
    }
} // namespace olympia::coreutils
