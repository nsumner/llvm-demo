#ifndef METADATACRAWLER_HPP
#define METADATACRAWLER_HPP

#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Constants.h"

#include <map>
#include <set>

#include "Util.hpp"

using namespace llvm;
using namespace std;

#define CHECK_TAG(x, tag) (((x) & (tag)) == (tag))

class MetadataCrawler;

class MDTraversal {
    protected:
        MetadataCrawler *mdc;
    public:
        MDTraversal(MetadataCrawler *a_mdc) : mdc(a_mdc) {};
        virtual void traverse(MDNode *) = 0;
};

class FieldNameTraversal : public MDTraversal {
    public:
        FieldNameTraversal(MetadataCrawler *a_mdc) : MDTraversal(a_mdc) {};
        void traverse(MDNode *); 
};

class TypedefTraversal : public MDTraversal {
    public:
        TypedefTraversal(MetadataCrawler *a_mdc) : MDTraversal(a_mdc) {};
        void traverse(MDNode *);
};


class MetadataCrawler {
    private:
        set<MDNode *> visited;
        void crawlAllInstructions(Module &m, MDTraversal *mdt);
        void crawlNMD(NamedMDNode *nmd, MDTraversal *mdt);

        TypedefTraversal typedefTraversal;
        FieldNameTraversal fieldNameTraversal;

    public:
        map<MDNode *, string> typedefMap;
        map<string, vector<string>> fieldMap;
        MetadataCrawler() : typedefTraversal(this), fieldNameTraversal(this) {};

        void traverseMetadata(MDNode *mdn, MDTraversal *traversal);
        void crawlModule(Module &m);
        void getFields(string className, MDNode *fields);
        string getFieldName(string structName, int fieldIdx);
};

#endif

