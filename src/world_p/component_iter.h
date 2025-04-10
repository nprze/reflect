#pragma once
#include "components.h"
namespace rfct {
	/*
	class Archetype;
	class ComponentIteratorBase {
	public:
		virtual ~ComponentIteratorBase() = default;
		virtual void* current() = 0;
		virtual bool hasNext() const = 0;
		virtual void next() = 0;
	};

	template<typename T>
	class ComponentIterator : public ComponentIteratorBase {
	public:
		ComponentIterator(const std::vector<Archetype*>& archetypes, ComponentEnum type);
			
		void* current() override;

		bool hasNext() const override;

		void next() override;

	private:
		void findNextNonEmpty();
		std::vector<Archetype*> archetypes_;
		size_t archetypeIndex_ = 0;
		ComponentEnum type_;
		std::vector<T>* currentVec_ = nullptr;
		size_t index_ = 0;
	};
	*/
}