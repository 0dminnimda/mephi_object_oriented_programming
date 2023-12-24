
class Shield : public Wearable {
public:
    DeepCopy(Shield);

    Shield() = default;
    Shield(size_t item_class_index, RangeOfLong defence_range)
        : Wearable(item_class_index, Wearable::Shield, defence_range) {}

    std::shared_ptr<Item> deepcopy_item() const override;

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Wearable);
    }
};

BOOST_CLASS_EXPORT_KEY(Shield);

void Shield::deepcopy_to(Shield &other) const { Wearable::deepcopy_to(other); }

std::shared_ptr<Item> Shield::deepcopy_item() const { return deepcopy_shared(*this); }
