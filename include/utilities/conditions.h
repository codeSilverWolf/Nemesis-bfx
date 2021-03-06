#pragma once

#include <map>
#include <memory>

#include <unordered_map>
#include <vector>

#include "utilities/scope.h"

#include "generate/animation/optionlist.h"
#include "generate/animation/animationinfo.h"

struct AddOnInfo;
struct AnimThreadInfo;

namespace nemesis
{
	struct Condt;
}

void GetMultiFromAddOn(const nemesis::Condt& curcond,
                       const AnimThreadInfo& animthrinfo,
					   const AddOnInfo& addinfo,
                       const std::string& original,
                       int animMulti,
                       int& optionMulti,
                       int& endMulti);

namespace nemesis
{
	struct Condt
	{
        friend Condt;
        friend void ::GetMultiFromAddOn(
            const Condt&, const AnimThreadInfo&, const AddOnInfo&, const std::string&, int, int&, int&);

	private:
		int hiddenOrder;
		bool last = false;

		const proc* prcs;
		std::string fmt;
		std::string bhvfile;
		int nm;
		bool ig;
		bool im;

	public:
		bool oneTime = false;
		bool isNot = false;
		bool OR = false;
		bool isOrder = false;

		std::string original;

		size_t cmp1_high;
		size_t cmp2_high;

		VecStr cmp1;
		VecStr cmp2;

		bool cmpbool1 = false;
		bool cmpbool2 = false;

		std::vector<AddOnInfo> cmpinfo1;
		std::vector<AddOnInfo> cmpinfo2;

		std::map<int, std::vector<std::shared_ptr<nemesis::scope>>> cmp1_block;
		std::map<int, std::vector<std::shared_ptr<nemesis::scope>>> cmp2_block;

		std::shared_ptr<VecStr> optioncondt;
		std::shared_ptr<Condt> nestedcond;
		std::shared_ptr<Condt> next;

		VecStr olddata;
		std::vector<int> olddataint;

		Condt(std::string condition, std::string format, std::string behaviorFile, std::string originalCondition, std::string multiOption, int numline, bool isGroup,
			bool isMaster, OptionList& optionlist);

		bool isTrue(Condt& condt, AnimThreadInfo& animthrinfo);
        bool isTrue(AnimThreadInfo& animthrinfo,
                    const proc& process,
                    std::string format,
                    std::string behaviorFile,
                    int numline,
                    bool isGroup,
                    bool isMaster);
        bool isMultiTrue(AnimThreadInfo& animthrinfo,
						 const proc& process,
                         std::string format,
                         std::string behaviorFile,
                         int numline,
                         int& animMulti,
                         bool isGroup,
                         bool isMaster,
                         int& groupMulti);

	private:
        bool specialIsTrueA(AnimThreadInfo& animthrinfo);
        bool specialIsTrueB(AnimThreadInfo& animthrinfo);

		void specialCondition(std::string condition, std::string format, std::string behaviorFile, std::string multiOption, int numline, bool isGroup, bool isMaster,
			OptionList& optionlist);
		void singleCondition(std::string condition, std::string format, std::string behaviorFile, std::string multiOption, int numline, bool isGroup, bool isMaster,
			OptionList& optionlist);
		void conditionProcess(std::string condition, std::string format, std::string behaviorFile, std::string multiOption, int numline, bool isGroup, bool isMaster);
	};

	struct MultiChoice
	{
		bool chosen = false;
		std::shared_ptr<Condt> condition;
		size_t locateA;
		size_t locateB;

		MultiChoice() {}
		MultiChoice(std::string cond, std::string format, std::string behaviorFile, std::string multiOption, int numline, bool isGroup, bool isMaster,
			OptionList& optionlist, size_t posA, size_t posB);
	};
}
