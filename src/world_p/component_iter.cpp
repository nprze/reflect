#include "component_iter.h"
namespace rfct {
	/*

	template<typename T>
	ComponentIterator<T>::ComponentIterator(const std::vector<Archetype*>& archetypes, ComponentEnum type) : archetypes_(archetypes), type_(type) {
		findNextNonEmpty();
	}
	template<typename T>
	void* ComponentIterator<T>::current() {
		if (hasNext()) {
			return &(*currentVec_)[index_];
		}
		return nullptr;
	}
	template<typename T>
	bool ComponentIterator<T>::hasNext() const {
		return currentVec_ != nullptr && index_ < currentVec_->size();
	}
	template<typename T>
	void ComponentIterator<T>::next() {
		if (!hasNext()) return;
		++index_;
		if (index_ >= currentVec_->size()) {
			++archetypeIndex_;
			findNextNonEmpty();
		}
	}
	template<typename T>
	void ComponentIterator<T>::findNextNonEmpty() {
		currentVec_ = nullptr;
		index_ = 0;
		while (archetypeIndex_ < archetypes_.size()) {
			auto it = archetypes_[archetypeIndex_]->componentMap.find(type_);
			if (it != archetypes_[archetypeIndex_]->componentMap.end()) {
				currentVec_ = static_cast<std::vector<T>*>(it->second);
				if (!currentVec_->empty()) return;
			}
			++archetypeIndex_;
		}
	}*/
}